
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/compiler.h>
#include <linux/rbtree.h>
#include <linux/interrupt.h>

/*
 * max time we may wait to anticipate a read (default around 6ms)
 */
//#define default_antic_expire ((HZ / 150) ? HZ / 150 : 1)
#define default_antic_expire 1
/*
 * Keep track of up to 20ms thinktimes. We can go as big as we like here,
 * however huge values tend to interfere and not decay fast enough. A program
 * might be in a non-io phase of operation. Waiting on user input for example,
 * or doing a lengthy computation. A small penalty can be justified there, and
 * will still catch out those processes that constantly have large thinktimes.
 */
#define MAX_THINKTIME (HZ/200UL)

/* Bits in as_io_context.state */
enum as_io_states {
	AS_TASK_RUNNING = 0,	/* Process has not exited */
	AS_TASK_IOSTARTED,	/* Process has started some IO */
	AS_TASK_IORUNNING,	/* Process has completed some IO */
};

enum anticipation_status {
	ANTIC_OFF=0,		/* Not anticipating (normal operation)	*/
	ANTIC_WAIT_REQ,		/* The last read has not yet completed  */
	ANTIC_WAIT_NEXT,	/* Currently anticipating a request vs
				   last read (which has completed) */
	ANTIC_FINISHED,		/* Anticipating but have found a candidate
				 * or timed out */
};

struct simple_data {
	struct request_queue *q;	/* the "owner" queue */
	struct rb_root sort_list;
	sector_t last_sector;

	unsigned long exit_prob;	/* probability a task will exit while
					   being waited on */
	unsigned long exit_no_coop;	/* probablility an exited task will
					   not be part of a later cooperating
					   request */
	struct request *next_rq;
	struct io_context *io_context;	/* Identify the expected process */
	struct timer_list antic_timer;	/* anticipatory scheduling timer */
	struct work_struct antic_work;	/* Deferred unplugging */
	unsigned long antic_start;	/* jiffies: when it started */	
	enum anticipation_status antic_status;

	int ioc_finished; /* IO associated with io_context is finished */
	int nr_dispatched;
	unsigned long new_ttime_total; 	/* mean thinktime on new proc */
	unsigned long new_ttime_mean;
	u64 new_seek_total;		/* mean seek on new proc */
	sector_t new_seek_mean;

	unsigned long antic_expire;
};

/*
 * per-request data.
 */
enum arq_state {
	AS_RQ_NEW=0,		/* New - not referenced and not on any lists */
	AS_RQ_QUEUED,		/* In the request queue. It belongs to the
				   scheduler */
	AS_RQ_DISPATCHED,	/* On the dispatch list. It belongs to the
				   driver now */
	AS_RQ_PRESCHED,		/* Debug poisoning for requests being used */
	AS_RQ_REMOVED,
	AS_RQ_MERGED,
	AS_RQ_POSTSCHED,	/* when they shouldn't be */
};

#define RQ_IOC(rq)	((struct io_context *) (rq)->elevator_private)
#define RQ_STATE(rq)	((enum arq_state)(rq)->elevator_private2)
#define RQ_SET_STATE(rq, state)	((rq)->elevator_private2 = (void *) state)


static DEFINE_PER_CPU(unsigned long, as_ioc_count);
static struct completion *ioc_gone;
static DEFINE_SPINLOCK(ioc_gone_lock);

static void simple_move_to_dispatch(struct simple_data *, struct request *rq);
static void simple_antic_stop(struct simple_data *);

/*
 * IO Context helper functions
 */

/* Called to deallocate the as_io_context */
static void free_as_io_context(struct as_io_context *aic)
{
	kfree(aic);
	elv_ioc_count_dec(as_ioc_count);
	if (ioc_gone) {
		/*
		 * AS scheduler is exiting, grab exit lock and check
		 * the pending io context count. If it hits zero,
		 * complete ioc_gone and set it back to NULL.
		 */
		spin_lock(&ioc_gone_lock);
		if (ioc_gone && !elv_ioc_count_read(as_ioc_count)) {
			complete(ioc_gone);
			ioc_gone = NULL;
		}
		spin_unlock(&ioc_gone_lock);
	}
}

static void simple_trim(struct io_context *ioc)
{
	spin_lock_irq(&ioc->lock);
	if (ioc->aic)
		free_as_io_context(ioc->aic);
	ioc->aic = NULL;
	spin_unlock_irq(&ioc->lock);
}

/* Called when the task exits */
static void exit_as_io_context(struct as_io_context *aic)
{
	WARN_ON(!test_bit(AS_TASK_RUNNING, &aic->state));
	clear_bit(AS_TASK_RUNNING, &aic->state);
}

static struct as_io_context *alloc_as_io_context(void)
{
	struct as_io_context *ret;

	ret = kmalloc(sizeof(*ret), GFP_ATOMIC);
	if (ret) {
		ret->dtor = free_as_io_context;
		ret->exit = exit_as_io_context;
		ret->state = 1 << AS_TASK_RUNNING;
		atomic_set(&ret->nr_queued, 0);
		atomic_set(&ret->nr_dispatched, 0);
		spin_lock_init(&ret->lock);
		ret->ttime_total = 0;
		ret->ttime_samples = 0;
		ret->ttime_mean = 0;
		ret->seek_total = 0;
		ret->seek_samples = 0;
		ret->seek_mean = 0;
		elv_ioc_count_inc(as_ioc_count);
	}

	return ret;
}

/*
 * If the current task has no AS IO context then create one and initialise it.
 * Then take a ref on the task's io context and return it.
 */
static struct io_context *simple_get_io_context(int node)
{
	struct io_context *ioc = get_io_context(GFP_ATOMIC, node);
	if (ioc && !ioc->aic) {
		ioc->aic = alloc_as_io_context();
		if (!ioc->aic) {
			put_io_context(ioc);
			ioc = NULL;
		}
	}
	return ioc;
}

static void simple_put_io_context(struct request *rq)
{
	struct as_io_context *aic;

	if (unlikely(!RQ_IOC(rq)))
		return;

	aic = RQ_IOC(rq)->aic;

	if (rq_is_sync(rq) && aic) {
		unsigned long flags;

		spin_lock_irqsave(&aic->lock, flags);
		set_bit(AS_TASK_IORUNNING, &aic->state);
		aic->last_end_request = jiffies;
		spin_unlock_irqrestore(&aic->lock, flags);
	}

	put_io_context(RQ_IOC(rq));
}

/*
 * rb tree support functions
 */
#define RQ_RB_ROOT(sd, rq)	(&(sd)->sort_list)

static void
simple_add_rq_rb(struct simple_data *sd, struct request *rq)
{
	struct rb_root *root = &sd->sort_list;
	struct request *__alias;

	while (unlikely(__alias = elv_rb_add(root, rq))) {
		simple_move_to_dispatch(sd, __alias);
		simple_antic_stop(sd);
	}
}

static inline void
simple_del_rq_rb(struct simple_data *sd, struct request *rq)
{
	elv_rb_del(&sd->sort_list, rq);
}


/*
 * IO Scheduler proper
 */

#define MAXBACK (1024 * 1024)	/*
				 * Maximum distance the disk will go backward
				 * for a request.
				 */

#define BACK_PENALTY	2

/*
 * as_choose_req selects the preferred one of two requests of the same data_dir
 * ignoring time - eg. timeouts, which is the job of as_dispatch_request
 */
static struct request *
simple_choose_req(struct simple_data *sd, struct request *rq1, struct request *rq2)
{
	sector_t last, s1, s2, d1, d2;
	int r1_wrap=0, r2_wrap=0;	/* requests are behind the disk head */
	const sector_t maxback = MAXBACK;

	if (rq1 == NULL || rq1 == rq2)
		return rq2;
	if (rq2 == NULL)
		return rq1;

	last = sd->last_sector;
	s1 = blk_rq_pos(rq1);
	s2 = blk_rq_pos(rq2);

	/*
	 * Strict one way elevator _except_ in the case where we allow
	 * short backward seeks which are biased as twice the cost of a
	 * similar forward seek.
	 */
	if (s1 >= last)
		d1 = s1 - last;
	else if (s1 + maxback >= last)
		d1 = (last - s1) * BACK_PENALTY;
	else {
		r1_wrap = 1;
		d1 = 0; /* shut up, gcc */
	}

	if (s2 >= last)
		d2 = s2 - last;
	else if (s2 + maxback >= last)
		d2 = (last - s2) * BACK_PENALTY;
	else {
		r2_wrap = 1;
		d2 = 0;
	}

	/* Found required data */
	if (!r1_wrap && r2_wrap)
		return rq1;
	else if (!r2_wrap && r1_wrap)
		return rq2;
	else if (r1_wrap && r2_wrap) {
		/* both behind the head */
		if (s1 <= s2)
			return rq1;
		else
			return rq2;
	}

	/* Both requests in front of the head */
	if (d1 < d2)
		return rq1;
	else if (d2 < d1)
		return rq2;
	else {
		if (s1 >= s2)
			return rq1;
		else
			return rq2;
	}
}

/*
 * as_find_next_rq finds the next request after @prev in elevator order.
 * this with as_choose_req form the basis for how the scheduler chooses
 * what request to process next. Anticipation works on top of this.
 */
static struct request *
simple_find_next_rq(struct simple_data *sd, struct request *last)
{
	struct rb_node *rbnext = rb_next(&last->rb_node);
	struct rb_node *rbprev = rb_prev(&last->rb_node);
	struct request *next = NULL, *prev = NULL;

	BUG_ON(RB_EMPTY_NODE(&last->rb_node));

	if (rbprev)
		prev = rb_entry_rq(rbprev);

	if (rbnext)
		next = rb_entry_rq(rbnext);
	else {
		rbnext = rb_first(&sd->sort_list);
		if (rbnext && rbnext != &last->rb_node)
			next = rb_entry_rq(rbnext);
	}

	return simple_choose_req(sd, next, prev);
}

/*
 * anticipatory scheduling functions follow
 */

/*
 * as_antic_expired tells us when we have anticipated too long.
 * The funny "absolute difference" math on the elapsed time is to handle
 * jiffy wraps, and disks which have been idle for 0x80000000 jiffies.
 */
static int simple_antic_expired(struct simple_data *sd)
{
	long delta_jif;

	delta_jif = jiffies - sd->antic_start;
	if (unlikely(delta_jif < 0))
		delta_jif = -delta_jif;
	if (delta_jif < sd->antic_expire)
		return 0;

	return 1;
}

/*
 * as_antic_waitnext starts anticipating that a nice request will soon be
 * submitted. See also as_antic_waitreq
 */
static void simple_antic_waitnext(struct simple_data *sd)
{
	unsigned long timeout;

	BUG_ON(sd->antic_status != ANTIC_OFF
			&& sd->antic_status != ANTIC_WAIT_REQ);

	timeout = sd->antic_start + sd->antic_expire;

	mod_timer(&sd->antic_timer, timeout);

	sd->antic_status = ANTIC_WAIT_NEXT;
}

/*
 * as_antic_waitreq starts anticipating. We don't start timing the anticipation
 * until the request that we're anticipating on has finished. This means we
 * are timing from when the candidate process wakes up hopefully.
 */
static void simple_antic_waitreq(struct simple_data *sd)
{
	BUG_ON(sd->antic_status == ANTIC_FINISHED);
	if (sd->antic_status == ANTIC_OFF) {
		if (!sd->io_context || sd->ioc_finished)
			simple_antic_waitnext(sd);
		else
			sd->antic_status = ANTIC_WAIT_REQ;
	}
}

/*
 * This is called directly by the functions in this file to stop anticipation.
 * We kill the timer and schedule a call to the request_fn asap.
 */
static void simple_antic_stop(struct simple_data *sd)
{
	int status = sd->antic_status;

	if (status == ANTIC_WAIT_REQ || status == ANTIC_WAIT_NEXT) {
		if (status == ANTIC_WAIT_NEXT)
			del_timer(&sd->antic_timer);
		sd->antic_status = ANTIC_FINISHED;
		/* see as_work_handler */
		kblockd_schedule_work(sd->q, &sd->antic_work);
	}
}

/*
 * as_antic_timeout is the timer function set by as_antic_waitnext.
 */
static void simple_antic_timeout(unsigned long data)
{
	struct request_queue *q = (struct request_queue *)data;
	struct simple_data *sd = q->elevator->elevator_data;
	unsigned long flags;

	spin_lock_irqsave(q->queue_lock, flags);
	if (sd->antic_status == ANTIC_WAIT_REQ
			|| sd->antic_status == ANTIC_WAIT_NEXT) {
		struct as_io_context *aic;
		spin_lock(&sd->io_context->lock);
		aic = sd->io_context->aic;

		sd->antic_status = ANTIC_FINISHED;
		kblockd_schedule_work(q, &sd->antic_work);

		if (aic->ttime_samples == 0) {
			/* process anticipated on has exited or timed out*/
			sd->exit_prob = (7*sd->exit_prob + 256)/8;
		}
		if (!test_bit(AS_TASK_RUNNING, &aic->state)) {
			/* process not "saved" by a cooperating request */
			sd->exit_no_coop = (7*sd->exit_no_coop + 256)/8;
		}
		spin_unlock(&sd->io_context->lock);
	}
	spin_unlock_irqrestore(q->queue_lock, flags);
}

static void simple_update_thinktime(struct simple_data *sd, struct as_io_context *aic,
				unsigned long ttime)
{
	/* fixed point: 1.0 == 1<<8 */
	if (aic->ttime_samples == 0) {
		sd->new_ttime_total = (7*sd->new_ttime_total + 256*ttime) / 8;
		sd->new_ttime_mean = sd->new_ttime_total / 256;

		sd->exit_prob = (7*sd->exit_prob)/8;
	}
	aic->ttime_samples = (7*aic->ttime_samples + 256) / 8;
	aic->ttime_total = (7*aic->ttime_total + 256*ttime) / 8;
	aic->ttime_mean = (aic->ttime_total + 128) / aic->ttime_samples;
}

static void simple_update_seekdist(struct simple_data *sd, struct as_io_context *aic,
				sector_t sdist)
{
	u64 total;

	if (aic->seek_samples == 0) {
		sd->new_seek_total = (7*sd->new_seek_total + 256*(u64)sdist)/8;
		sd->new_seek_mean = sd->new_seek_total / 256;
	}

	/*
	 * Don't allow the seek distance to get too large from the
	 * odd fragment, pagein, etc
	 */
	if (aic->seek_samples <= 60) /* second&third seek */
		sdist = min(sdist, (aic->seek_mean * 4) + 2*1024*1024);
	else
		sdist = min(sdist, (aic->seek_mean * 4)	+ 2*1024*64);

	aic->seek_samples = (7*aic->seek_samples + 256) / 8;
	aic->seek_total = (7*aic->seek_total + (u64)256*sdist) / 8;
	total = aic->seek_total + (aic->seek_samples/2);
	do_div(total, aic->seek_samples);
	aic->seek_mean = (sector_t)total;
}

/*
 * as_update_iohist keeps a decaying histogram of IO thinktimes, and
 * updates @aic->ttime_mean based on that. It is called when a new
 * request is queued.
 */
static void simple_update_iohist(struct simple_data *sd, struct as_io_context *aic,
				struct request *rq)
{
	unsigned long thinktime = 0;
	sector_t seek_dist;
	unsigned long in_flight;

	if (aic == NULL)
		return;

	in_flight = atomic_read(&aic->nr_queued) + atomic_read(&aic->nr_dispatched);
	spin_lock(&aic->lock);
	if (test_bit(AS_TASK_IORUNNING, &aic->state) ||
		test_bit(AS_TASK_IOSTARTED, &aic->state)) {
		/* Calculate read -> read thinktime */
		if (test_bit(AS_TASK_IORUNNING, &aic->state)
						&& in_flight == 0) {
			thinktime = jiffies - aic->last_end_request;
			thinktime = min(thinktime, MAX_THINKTIME-1);
		}
		simple_update_thinktime(sd, aic, thinktime);

		/* Calculate read -> read seek distance */
		if (aic->last_request_pos < blk_rq_pos(rq))
			seek_dist = blk_rq_pos(rq) -
				    aic->last_request_pos;
		else
			seek_dist = aic->last_request_pos -
				    blk_rq_pos(rq);
		simple_update_seekdist(sd, aic, seek_dist);
	}
	aic->last_request_pos = blk_rq_pos(rq) + blk_rq_sectors(rq);
	set_bit(AS_TASK_IOSTARTED, &aic->state);
	spin_unlock(&aic->lock);
}

/*
 * as_close_req decides if one request is considered "close" to the
 * previous one issued.
 */
static int simple_close_req(struct simple_data *sd, struct as_io_context *aic,
			struct request *rq)
{
	unsigned long delay;	/* jiffies */
	sector_t last = sd->last_sector;
	sector_t next = blk_rq_pos(rq);
	sector_t delta; /* acceptable close offset (in sectors) */
	sector_t s;

	if (sd->antic_status == ANTIC_OFF || !sd->ioc_finished)
		delay = 0;
	else
		delay = jiffies - sd->antic_start;

	if (delay == 0)
		delta = 8192;
	else if (delay <= (20 * HZ / 1000) && delay <= sd->antic_expire)
		delta = 8192 << delay;
	else
		return 1;

	if ((last <= next + (delta>>1)) && (next <= last + delta))
		return 1;

	if (last < next)
		s = next - last;
	else
		s = last - next;

	if (aic->seek_samples == 0) {
		/*
		 * Process has just started IO. Use past statistics to
		 * gauge success possibility
		 */
		if (sd->new_seek_mean > s) {
			/* this request is better than what we're expecting */
			return 1;
		}

	} else {
		if (aic->seek_mean > s) {
			/* this request is better than what we're expecting */
			return 1;
		}
	}

	return 0;
}

/*
 * as_can_break_anticipation returns true if we have been anticipating this
 * request.
 *
 * It also returns true if the process against which we are anticipating
 * submits a write - that's presumably an fsync, O_SYNC write, etc. We want to
 * dispatch it ASAP, because we know that application will not be submitting
 * any new reads.
 *
 * If the task which has submitted the request has exited, break anticipation.
 *
 * If this task has queued some other IO, do not enter enticipation.
 */
static int simple_can_break_anticipation(struct simple_data *sd, struct request *rq)
{
	struct io_context *ioc;
	struct as_io_context *aic;

	ioc = sd->io_context;
	BUG_ON(!ioc);
	spin_lock(&ioc->lock);

	if (rq && ioc == RQ_IOC(rq)) {
		/* request from same process */
		spin_unlock(&ioc->lock);
		return 1;
	}

	if (sd->ioc_finished && simple_antic_expired(sd)) {
		/*
		 * In this situation status should really be FINISHED,
		 * however the timer hasn't had the chance to run yet.
		 */
		spin_unlock(&ioc->lock);
		return 1;
	}

	aic = ioc->aic;
	if (!aic) {
		spin_unlock(&ioc->lock);
		return 0;
	}

	if (atomic_read(&aic->nr_queued) > 0) {
		/* process has more requests queued */
		spin_unlock(&ioc->lock);
		return 1;
	}

	if (atomic_read(&aic->nr_dispatched) > 0) {
		/* process has more requests dispatched */
		spin_unlock(&ioc->lock);
		return 1;
	}

	if (rq && simple_close_req(sd, aic, rq)) {
		/*
		 * Found a close request that is not one of ours.
		 *
		 * This makes close requests from another process update
		 * our IO history. Is generally useful when there are
		 * two or more cooperating processes working in the same
		 * area.
		 */
		if (!test_bit(AS_TASK_RUNNING, &aic->state)) {
			if (aic->ttime_samples == 0)
				sd->exit_prob = (7*sd->exit_prob + 256)/8;

			sd->exit_no_coop = (7*sd->exit_no_coop)/8;
		}

		simple_update_iohist(sd, aic, rq);
		spin_unlock(&ioc->lock);
		return 1;
	}

	if (!test_bit(AS_TASK_RUNNING, &aic->state)) {
		/* process anticipated on has exited */
		if (aic->ttime_samples == 0)
			sd->exit_prob = (7*sd->exit_prob + 256)/8;

		if (sd->exit_no_coop > 128) {
			spin_unlock(&ioc->lock);
			return 1;
		}
	}

	if (aic->ttime_samples == 0) {
		if (sd->new_ttime_mean > sd->antic_expire) {
			spin_unlock(&ioc->lock);
			return 1;
		}
		if (sd->exit_prob * sd->exit_no_coop > 128*256) {
			spin_unlock(&ioc->lock);
			return 1;
		}
	} else if (aic->ttime_mean > sd->antic_expire) {
		/* the process thinks too much between requests */
		spin_unlock(&ioc->lock);
		return 1;
	}
	spin_unlock(&ioc->lock);
	return 0;
}

/*
 * as_can_anticipate indicates whether we should either run rq
 * or keep anticipating a better request.
 */
static int simple_can_anticipate(struct simple_data *sd, struct request *rq)
{
	if (!sd->io_context)
		/*
		 * Last request submitted was a write
		 */
		return 0;

	if (sd->antic_status == ANTIC_FINISHED)
		/*
		 * Don't restart if we have just finished. Run the next request
		 */
		return 0;

	if (simple_can_break_anticipation(sd, rq))
		/*
		 * This request is a good candidate. Don't keep anticipating,
		 * run it.
		 */
		return 0;

	/*
	 * OK from here, we haven't finished, and don't have a decent request!
	 * Status is either ANTIC_OFF so start waiting,
	 * ANTIC_WAIT_REQ so continue waiting for request to finish
	 * or ANTIC_WAIT_NEXT so continue waiting for an acceptable request.
	 */

	return 1;
}


/*
 * as_update_rq must be called whenever a request (rq) is added to
 * the sort_list. This function keeps caches up to date, and checks if the
 * request might be one we are "anticipating"
 */
static void simple_update_rq(struct simple_data *sd, struct request *rq)
{

	/* keep the next_rq cache up to date */
	sd->next_rq = simple_choose_req(sd, rq, sd->next_rq);

	/*
	 * have we been anticipating this request?
	 * or does it come from the same process as the one we are anticipating
	 * for?
	 */
	if (sd->antic_status == ANTIC_WAIT_REQ
			|| sd->antic_status == ANTIC_WAIT_NEXT) {
		if (simple_can_break_anticipation(sd, rq))
			simple_antic_stop(sd);
	}
}


/*
 * as_completed_request is to be called when a request has completed and
 * returned something to the requesting process, be it an error or data.
 */
static void simple_completed_request(struct request_queue *q, struct request *rq)
{
	struct simple_data *sd = q->elevator->elevator_data;
	if (RQ_STATE(rq) != AS_RQ_REMOVED) {
		goto out;
	}
	sd->nr_dispatched--;

	if (sd->io_context == RQ_IOC(rq) && sd->io_context) {
		sd->antic_start = jiffies;
		sd->ioc_finished = 1;
		if (sd->antic_status == ANTIC_WAIT_REQ) {
			/*
			 * We were waiting on this request, now anticipate
			 * the next one
			 */
			simple_antic_waitnext(sd);
		}
	}
	simple_put_io_context(rq);
out:
	RQ_SET_STATE(rq, AS_RQ_POSTSCHED);
}

static void simple_remove_queued_request(struct request_queue *q,
				     struct request *rq)
{
	struct simple_data *sd = q->elevator->elevator_data;
	struct io_context *ioc = RQ_IOC(rq);
	if (ioc && ioc->aic)
		atomic_dec(&ioc->aic->nr_queued);
	if (sd->next_rq == rq)
		sd->next_rq = simple_find_next_rq(sd, rq);
	simple_del_rq_rb(sd, rq);
}

static void simple_move_to_dispatch(struct simple_data *sd, struct request *rq)
{
	struct io_context *ioc = RQ_IOC(rq);
	simple_antic_stop(sd);
	sd->antic_status = ANTIC_OFF;
	/*
	 * This has to be set in order to be correctly updated by
	 * as_find_next_rq
	 */
	sd->last_sector = blk_rq_pos(rq) + blk_rq_sectors(rq);
	copy_io_context(&sd->io_context, &ioc);
	sd->ioc_finished = 0;
	sd->next_rq = simple_find_next_rq(sd, rq);
	/*
	 * take it off the sort and fifo list, add to dispatch queue
	 */
	simple_remove_queued_request(sd->q, rq);
	WARN_ON(RQ_STATE(rq) != AS_RQ_QUEUED);

	elv_dispatch_sort(sd->q, rq);

	RQ_SET_STATE(rq, AS_RQ_DISPATCHED);
	if (RQ_IOC(rq) && RQ_IOC(rq)->aic)
		atomic_inc(&RQ_IOC(rq)->aic->nr_dispatched);
	sd->nr_dispatched++;
}

static int
simple_dispatch_request(struct request_queue *q, int force)
{
	struct simple_data *sd = q->elevator->elevator_data;
	struct request *rq = sd->next_rq;
	if (unlikely(force)) {
		int dispatched = 0;
		while (sd->next_rq) {
			simple_move_to_dispatch(sd, sd->next_rq);
			dispatched++;
		}
		return dispatched;
	}
	// 如果队列为空，不发送
	if (RB_EMPTY_ROOT(&sd->sort_list))
		return 0;
	if (sd->antic_status == ANTIC_WAIT_REQ || sd->antic_status == ANTIC_WAIT_NEXT)
		return 0;
	if (sd->antic_expire) {
		if (simple_can_anticipate(sd, rq)) {
			simple_antic_waitreq(sd);
			return 0;
		}
	}
	simple_move_to_dispatch(sd, rq);
	return 1;
}

static void simple_add_request(struct request_queue *q, struct request *rq)
{
	struct simple_data *sd = q->elevator->elevator_data;

	RQ_SET_STATE(rq, AS_RQ_NEW);

	rq->elevator_private = simple_get_io_context(q->node);

	if (RQ_IOC(rq)) {
		simple_update_iohist(sd, RQ_IOC(rq)->aic, rq);
		atomic_inc(&RQ_IOC(rq)->aic->nr_queued);
	}

	simple_add_rq_rb(sd, rq);

	simple_update_rq(sd, rq); /* keep state machine up to date */
	RQ_SET_STATE(rq, AS_RQ_QUEUED);
}

static void simple_activate_request(struct request_queue *q, struct request *rq)
{
	WARN_ON(RQ_STATE(rq) != AS_RQ_DISPATCHED);
	RQ_SET_STATE(rq, AS_RQ_REMOVED);
	if (RQ_IOC(rq) && RQ_IOC(rq)->aic)
		atomic_dec(&RQ_IOC(rq)->aic->nr_dispatched);
}

static void simple_deactivate_request(struct request_queue *q, struct request *rq)
{
	WARN_ON(RQ_STATE(rq) != AS_RQ_REMOVED);
	RQ_SET_STATE(rq, AS_RQ_DISPATCHED);
	if (RQ_IOC(rq) && RQ_IOC(rq)->aic)
		atomic_inc(&RQ_IOC(rq)->aic->nr_dispatched);
}

/*
 * as_queue_empty tells us if there are requests left in the device. It may
 * not be the case that a driver can get the next request even if the queue
 * is not empty - it is used in the block layer to check for plugging and
 * merging opportunities
 */
static int simple_queue_empty(struct request_queue *q)
{
	struct simple_data *sd = q->elevator->elevator_data;
	return RB_EMPTY_ROOT(&sd->sort_list);
}

static int
simple_merge(struct request_queue *q, struct request **req, struct bio *bio)
{
	struct simple_data *sd = q->elevator->elevator_data;
	struct request *__rq;
	int ret;

	sector_t sector = bio->bi_sector + bio_sectors(bio);

	__rq = elv_rb_find(&sd->sort_list, sector);
	if (__rq) {
		BUG_ON(sector != blk_rq_pos(__rq));

		if (elv_rq_merge_ok(__rq, bio)) {
			ret = ELEVATOR_FRONT_MERGE;
			*req = __rq;
			return ret;
		}
	}

	return ELEVATOR_NO_MERGE;
}

static void simple_merged_request(struct request_queue *q, struct request *req,
			      int type)
{
	struct simple_data *sd = q->elevator->elevator_data;

	/*
	 * if the merge was a front merge, we need to reposition request
	 */
	if (type == ELEVATOR_FRONT_MERGE) {
		simple_del_rq_rb(sd, req);
		simple_add_rq_rb(sd, req);
		/*
		 * Note! At this stage of this and the next function, our next
		 * request may not be optimal - eg the request may have "grown"
		 * behind the disk head. We currently don't bother adjusting.
		 */
	}
}

static void simple_merged_requests(struct request_queue *q, struct request *req,
			 	struct request *next)
{
	simple_remove_queued_request(q, next);
	simple_put_io_context(next);
	RQ_SET_STATE(next, AS_RQ_MERGED);
}

/*
 * This is executed in a "deferred" process context, by kblockd. It calls the
 * driver's request_fn so the driver can submit that request.
 *
 * IMPORTANT! This guy will reenter the elevator, so set up all queue global
 * state before calling, and don't rely on any state over calls.
 *
 * FIXME! dispatch queue is not a queue at all!
 */
static void simple_work_handler(struct work_struct *work)
{
	struct simple_data *sd = container_of(work, struct simple_data, antic_work);

	blk_run_queue(sd->q);
}

static int simple_may_queue(struct request_queue *q, int rw)
{
	int ret = ELV_MQUEUE_MAY;
	struct simple_data *sd = q->elevator->elevator_data;
	struct io_context *ioc;
	if (sd->antic_status == ANTIC_WAIT_REQ ||
			sd->antic_status == ANTIC_WAIT_NEXT) {
		ioc = simple_get_io_context(q->node);
		if (sd->io_context == ioc)
			ret = ELV_MQUEUE_MUST;
		put_io_context(ioc);
	}

	return ret;
}

static void simple_exit_queue(struct elevator_queue *e)
{
	struct simple_data *sd = e->elevator_data;
	del_timer_sync(&sd->antic_timer);
	cancel_work_sync(&sd->antic_work);

	put_io_context(sd->io_context);
	kfree(sd);
}

static void *simple_init_queue(struct request_queue *q)
{
	struct simple_data *sd;
	sd = kmalloc_node(sizeof(*sd), GFP_KERNEL | __GFP_ZERO, q->node);
	if (!sd)
		return NULL;
	sd->q = q;

	/* anticipatory scheduling helpers */
	sd->antic_timer.function = simple_antic_timeout;
	sd->antic_timer.data = (unsigned long)q;
	init_timer(&sd->antic_timer);
	INIT_WORK(&sd->antic_work, simple_work_handler);

	sd->sort_list = RB_ROOT;
	sd->antic_expire = default_antic_expire;
	sd->last_sector = 0;
	sd->next_rq = NULL;
	return sd;
}

static struct elevator_type iosched_simple = {
	.ops = {
		.elevator_merge_fn = simple_merge,
		.elevator_merged_fn = simple_merged_request,
		.elevator_merge_req_fn = simple_merged_requests,
		.elevator_dispatch_fn = simple_dispatch_request,
		.elevator_add_req_fn = simple_add_request,
		.elevator_queue_empty_fn =	simple_queue_empty,
		.elevator_former_req_fn = elv_rb_former_request,
		.elevator_latter_req_fn = elv_rb_latter_request,
		.elevator_activate_req_fn = simple_activate_request,
		.elevator_deactivate_req_fn = simple_deactivate_request,
		.elevator_may_queue_fn = simple_may_queue,
		.elevator_completed_req_fn = simple_completed_request,
		.elevator_init_fn = simple_init_queue,
		.elevator_exit_fn = simple_exit_queue,
		.trim = simple_trim,
	},

	// .elevator_attrs = simple_attrs,
	.elevator_name = "simple",
	.elevator_owner = THIS_MODULE,
};

static int __init simple_init(void)
{
	elv_register(&iosched_simple);
	return 0;
}

static void __exit simple_exit(void)
{
	DECLARE_COMPLETION_ONSTACK(all_gone);
	elv_unregister(&iosched_simple);
	ioc_gone = &all_gone;
	/* ioc_gone's update must be visible before reading ioc_count */
	smp_wmb();
	if (elv_ioc_count_read(as_ioc_count))
		wait_for_completion(&all_gone);
	synchronize_rcu();
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_AUTHOR("Liu Shuo");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple IO scheduler");
