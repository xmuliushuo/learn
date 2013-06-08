
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

struct simple_data {
	struct rb_root sort_list;
	sector_t last_sector;
	struct request *next_rq;
};

/*
 * get the request after `rq' in sector-sorted order
 */
static inline struct request *
simple_latter_request(struct request *rq)
{
	struct rb_node *node = rb_next(&rq->rb_node);

	if (node)
		return rb_entry_rq(node);

	return NULL;
}

static void simple_dispatch_request(struct simple_data*, struct request *);

static void
simple_add_rq_rb(struct simple_data *sd, struct request *rq)
{
	struct rb_root *root = &sd->sort_list;
	struct request *__alias;

	while (unlikely(__alias = elv_rb_add(root, rq)))
		simple_dispatch_request(sd, __alias);
}

static inline void
simple_del_rq_rb(struct simple_data *sd, struct request *rq)
{
	if (sd->next_rq == rq)
		sd->next_rq = simple_latter_request(rq);

	elv_rb_del(&sd->sort_list, rq);
}

static void
simple_dispatch_request(struct simple_data *sd, struct request *rq)
{
	sd->next_rq = simple_latter_request(rq);

	sd->last_sector = rq_end_sector(rq);

	/*
	 * take it off the sort and fifo list, move
	 * to dispatch queue
	 */
	simple_del_rq_rb(sd, rq);
	elv_dispatch_add_tail(rq->q, rq);
}

static int simple_dispatch_requests(struct request_queue *q, int force)
{
	struct simple_data *sd = q->elevator->elevator_data;
	struct rb_root *root = &sd->sort_list;
	struct request *rq;
	struct rb_node *node;

	if (RB_EMPTY_ROOT(root))
		return 0;
	if (sd->next_rq == NULL) {
		node = rb_first(root);
		rq = rb_entry_rq(node);
	}
	else
		rq = sd->next_rq;

	simple_dispatch_request(sd, rq);

	return 1;
}

static void
simple_merged_requests(struct request_queue *q, struct request *req,
			 struct request *next)
{
	/*
	 * kill knowledge of next, this one is a goner
	 */
	simple_del_rq_rb(q->elevator->elevator_data, next);
}

static void simple_merged_request(struct request_queue *q,
				    struct request *req, int type)
{
	struct simple_data *sd = q->elevator->elevator_data;
	struct rb_root *root = &sd->sort_list;
	/*
	 * if the merge was a front merge, we need to reposition request
	 */
	if (type == ELEVATOR_FRONT_MERGE) {
		elv_rb_del(root, req);
		simple_add_rq_rb(sd, req);
	}
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
		//BUG_ON(sector != blk_rq_pos(__rq));

		if (elv_rq_merge_ok(__rq, bio)) {
			ret = ELEVATOR_FRONT_MERGE;
			goto out;
		}
	}

	return ELEVATOR_NO_MERGE;
out:
	*req = __rq;
	return ret;
}

/*
 * add rq to rbtree and fifo
 */
static void simple_add_request(struct request_queue *q, struct request *rq)
{
	struct simple_data *sd = q->elevator->elevator_data;
	printk("simple add request\n");
	simple_add_rq_rb(sd, rq);
}

static int simple_queue_empty(struct request_queue *q)
{
	struct simple_data *sd = q->elevator->elevator_data;
	printk("simple_queue_emtpy\n");
	return RB_EMPTY_ROOT(&sd->sort_list);
}

static void simple_exit_queue(struct elevator_queue *e)
{
	struct simple_data *sd = e->elevator_data;
	printk("simple exit queue\n");

	kfree(sd);
}

static void *simple_init_queue(struct request_queue *q)
{
	struct simple_data *sd;
	printk("simple init queue\n");
	sd = kmalloc_node(sizeof(*sd), GFP_KERNEL | __GFP_ZERO, q->node);
	if (!sd)
		return NULL;

	sd->sort_list = RB_ROOT;
	sd->last_sector = 0;
	sd->next_rq = NULL;
	return sd;
}

static struct elevator_type iosched_simple = {
	.ops = {
		.elevator_merge_fn = simple_merge,
		.elevator_merged_fn = simple_merged_request,
		.elevator_merge_req_fn = simple_merged_requests,
		.elevator_dispatch_fn = simple_dispatch_requests,
		.elevator_add_req_fn = simple_add_request,
		.elevator_queue_empty_fn =	simple_queue_empty,
		.elevator_former_req_fn = elv_rb_former_request,
		.elevator_latter_req_fn = elv_rb_latter_request,
		.elevator_init_fn = simple_init_queue,
		.elevator_exit_fn = simple_exit_queue,
	},

	// .elevator_attrs = simple_attrs,
	.elevator_name = "simple",
	.elevator_owner = THIS_MODULE,
};

static int __init simple_init(void)
{
	printk(KERN_INFO "simple_init\n");
	elv_register(&iosched_simple);
	return 0;
}

static void __exit simple_exit(void)
{
	printk(KERN_INFO "simple_exit\n");
	elv_unregister(&iosched_simple);
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_AUTHOR("Liu Shuo");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple IO scheduler");
