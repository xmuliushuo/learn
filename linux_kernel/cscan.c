// CSCAN磁盘调度算法
// 使用两个队列，按照块号由小到大排列，其中一个为当前队列。
// 当一个磁盘请求到达时，检查该请求的块号是否大于当前队列的最小块号，
// 若大于最小块号，则加入当前队列，否则加入另外一个队列，
// 不停地完成当前队列的请求直到队列为空，然后互换两个队列的角色。
// 刘硕

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


#define RB_NONE (2)

struct cscan_request {
	struct request* request;   
	unsigned char queue_id; /* Which queue is this request on */
	struct rb_node rb_node;
	sector_t rb_key;
};

struct cscan_data {
	struct rb_root sort_list[2];
	sector_t last_sector;	
	unsigned int curr;
	unsigned int count[2];
};

static struct kmem_cache * __crq_pool;
static mempool_t *crq_pool;

static inline struct cscan_request* 
rb_insert_cscan_request(struct rb_root *root,	struct cscan_request *crq)
{
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct cscan_request * __crq;

	while (*p) {
		parent = *p;
		__crq = rb_entry(parent, struct cscan_request, rb_node);
		if (crq->rb_key < __crq->rb_key)
			p = &(*p)->rb_left;
		else if (crq->rb_key > __crq->rb_key)
			p = &(*p)->rb_right;
		else {
			printk(KERN_ALERT "maybe a bug here");
			return __crq;
		}
	}
	rb_link_node(&crq->rb_node, parent, p);
	return NULL;
}

static inline struct
cscan_request * rb_find_cscan_request(struct rb_root * root, long key) 
{
	struct rb_node * n = root->rb_node;
	struct cscan_request * crq;

	while(n) {
		crq = rb_entry(n,struct cscan_request,rb_node);
		if (key > crq->rb_key) {
			n = n->rb_right;
		} else if (key < crq->rb_key) {
			n = n->rb_left;
		} else {
			return crq;
		}
	}
	return NULL;
}

// int elevator_merge_fn(struct request_queue *queue, struct request **req, struct bio *bio);
// Description
// Discover an existing request that a bio can be merged with.
// Returns the type of merge discovered. One of:
// ELEVATOR_BACK_MERGE
// The bio can be merged into the back of the chosen request.
// ELEVATOR_FRONT_MERGE
// The bio can be merged into the front of the chosen request.
// ELEVATOR_NO_MERGE
// No suitable merge candidate was found.
// The request to merge into is returned via req.
// Arguments
// req: The existing request chosen to merge bio into.
// bio: The bio that will be merged into the existing request.
// static int 
// cscan_merge(request_queue_t *q, struct request **req, struct bio *bio)
// {
// 	int ret;
// 	struct list_head *entry;
// 	struct request *__rq;
// 	struct cscan_data *cd = q->elevator->elevator_data;
// 	/*
// 	 * try last_merge to avoid going to hash
// 	 */
// 	ret = elv_try_last_merge(q, bio);
// 	if (ret != ELEVATOR_NO_MERGE) {
// 		*req = q->last_merge;
// 		return ret;
// 	}
// 	entry = &cd->sort_list[curr];
// 	while ((entry = entry->prev) != &cd->sort_list[curr]) {
// 		__rq = list_entry(entry, struct cscan_request, list);
// 		if ((ret = elv_try_merge(__rq, bio))) {
// 			*req = __rq;
// 			q->last_merge = __rq;
// 			return ret;
// 		}
// 	}
// 	entry = &cd->sort_list[1 - curr];
// 	while ((entry = entry->prev) != &cd->sort_list[1 - curr]) {
// 		__rq = list_entry(entry, struct cscan_request, list);
// 		if ((ret = elv_try_merge(__rq, bio))) {
// 			*req = __rq;
// 			q->last_merge = __rq;
// 			return ret;
// 		}
// 	}
// 	return ELEVATOR_NO_MERGE;
// }

// int elevator_dispatch_fn(struct request_queue *queue, int force);
// Description
// Requests the scheduler to populate the dispatch queue with requests. 
// Once requests have been dispatched, the scheduler may not manipulate them.
// Returns the number of requests dispatched.
// This function is mandatory.
// Arguments
// force
// If non-zero, requests must be dispatched, regardless of scheduling policy.
// This is used, for instance, to drain requests before switching schedulers. 
// If 0, as in normal operation, requests may be postponed.
static int cscan_dispatch_requests(struct request_queue *q, int force)
{
	struct cscan_data *cd = (struct cscan_data *)q->elevator->elevator_data;
	struct rb_root *root = NULL;
	struct cscan_request *crq = NULL;
	struct request *rq = NULL;
	if (rb_first(&cd->sort_list[cd->curr])) {
		root = &cd->sort_list[cd->curr]; 
	}
	else if (rb_first(&cd->sort_list[1 - cd->curr])) {
		root = &cd->sort_list[1 - cd->curr]; 
		cd->curr = 1 - cd->curr; 
	}
	if (root) {
		crq = rb_entry(rb_first(root), struct cscan_request, rb_node);
		rq = crq->request;
		rb_erase(&crq->rb_node, &cd->sort_list[crq->queue_id]);
		// crq->rb_node.rb_color = RB_NONE;
		cd->last_sector = rq->sector + rq->nr_sectors;
		cd->count[crq->queue_id]--;
		elv_dispatch_sort(q, rq);
		return 1;
	}
	return 0;
}

// void elevator_merge_req_fn(struct request_queue *queue, struct request *req1, 
// 		struct request *req2);
// Description
// Called when two requests have been coalesced into a single request. 
// This occurs when a request grows (as a result of a bio merge) to become 
// adjacent with an existing request.
// Arguments
// req1: This request will grow to accommodate req2.
// req2: This request is coalsced with req1 and will be deallocated.
void cscan_merged_requests(struct request_queue *q, 
	struct request *req1, struct request *req2)
{
	struct cscan_data *cd = (struct cscan_data *)q->elevator->elevator_data;
	struct cscan_request *crq = (struct cscan_request *)req2->elevator_private;
	BUG_ON(!crq);
	rb_erase(&crq->rb_node, &cd->sort_list[crq->queue_id]);
	// crq->rb_node.rb_color = RB_NONE;
	cd->count[crq->queue_id]--;
}

static void
cscan_add_crq_rb(struct cscan_data * cd, struct cscan_request * crq) 
{
	crq->rb_key = crq->request->sector;
	rb_insert_cscan_request(&cd->sort_list[crq->queue_id], crq);
	rb_insert_color(&crq->rb_node, &cd->sort_list[crq->queue_id]);
	cd->count[crq->queue_id]++;
}

// void elevator_add_req_fn(struct request_queue *queue, struct request *req);
// Description
// Queues a new request with the scheduler.
// This function is mandatory.
// Arguments
// req: The request to be enqueued.
static void cscan_add_request(struct request_queue *q, struct request *rq) {
	struct cscan_data *cd = (struct cscan_data *)q->elevator->elevator_data;
	struct cscan_request *crq = (struct cscan_request*)rq->elevator_private;

	if (rq->sector > cd->last_sector) {
		crq->queue_id = cd->curr;
	}
	else {
		crq->queue_id = 1 - cd->curr;
	}
	cscan_add_crq_rb(cd, crq);
	if(rq_mergeable(rq)) {
		if(!q->last_merge)
			q->last_merge = rq;
	}
}

static int cscan_queue_empty(struct request_queue *q)
{
	struct cscan_data *cd = (struct cscan_data *)q->elevator->elevator_data;
	return ((rb_first(&cd->sort_list[cd->curr]) == NULL) && 
		(rb_first(&cd->sort_list[1 - cd->curr]) == NULL));
}

// int elevator_set_req_fn(struct request_queue *queue,
// 		struct request *req, gfp_t gfp);
// Description
// Allocate scheduler-private data for a request.
// Called when allocating request.
// Returns 0 if allocation succeeds, non-zero otherwise.
// Arguments
// req
// The request being allocated.
// gfp
// Modifier required for any allocations performed.
static int 
cscan_set_request(struct request_queue *q, struct request *rq, gfp_t gfp_mask) 
{
	struct cscan_request *crq;

	// void *mempool_alloc(mempool_t *pool, int gfp_mask);
	// void mempool_free(void *element, mempool_t *pool);
	// 在创建mempool时，分配函数将被调用多次来创建预先分配的对象。
	// 因此，对 mempool_alloc 的调用是试图用分配函数请求额外的对象，如果失败，则返回预先分配的对象(如果存在)。
	// 用 mempool_free 释放对象时，若预分配的对象数目小于最小量，就将它保留在池中，否则将它返回给系统。
	//
	// gfp_mask标志在alloc_page，kmalloc函数中都有用到。该标志可以分为三类：行为修饰符，区修饰符，类型修饰符。
	// 行为修饰符表示内核应当如何分配所需的内存，在某些特定的情况下，只能使用某些特定的方法分配内存，
	// 例如，中断处理程序就要求内核在分配内存时不能睡眠（因为中断处理程序不能被重新调度）。
	// 常用标志值有__GPF_WAIT(分配器可以睡眠)，__GFP_IO(分配器可以启动磁盘I/O)，
	// __GFP_FS(分配器可以启动文件系统I/O)等等。
	// 区修饰符表示从哪个区分配内存，内核优先从ZONE_NORMAL开始分配。区修饰符的值只有__GFP_DMA，__GFP_HIGHMEM。
	// 类型修饰符指定所需的行为和区描述符以完成特殊类型的处理。
	// 最常用的值为__GFP_KERNEL （__GFP_WAIT | __GFP_IO | __GFP_FS）
	crq = mempool_alloc(crq_pool, gfp_mask);
	if(crq) {
		memset(crq, 0, sizeof(*crq));
		crq->request = rq;
		// TODO RB_CLEARNODE
		// crq->rb_node.rb_color = RB_NONE;
		rq->elevator_private = crq;
		return 0;
	}
	return 1;
}

// void elevator_put_req_fn(struct request *req);
// Description
// Free scheduler-private data for a request allocated in elevator_set_req_fn.
// Arguments
// req
// The request owning the private data to be freed.
static void cscan_put_request(struct request *rq) 
{
	struct cscan_request *crq = (struct cscan_request*)rq->elevator_private;
	if(crq) {
		mempool_free(crq, crq_pool);
		rq->elevator_private = NULL;
	}
}
// void elevator_exit_fn(elevator_t *elv);
// Description
// Free any resources allocated in elevator_init_fn.
// No requests should be outstanding when called.
// Arguments
// elv
// The elevator instance to be freed.
static void cscan_exit_queue(elevator_t * e) 
{
	struct cscan_data *cd = (struct cscan_data *)e->elevator_data;
	BUG_ON(rb_first(&cd->sort_list[cd->curr]) != NULL);
	BUG_ON(rb_first(&cd->sort_list[1 - cd->curr]) != NULL);
	kfree(cd);
}

// void *elevator_init_fn(struct request_queue *queue);
// Description
// Initialise a new I/O scheduler instance. In particular,
// allocate and initialise any private data.
// Returns a pointer to the elevator instance private data,
// or NULL if initialisation fails.
// This function is mandatory.
// Arguments
// queue
// The queue this I/O scheduler will be attached to.
static void *cscan_init_queue(struct request_queue *q) 
{
	struct cscan_data * cd;
	cd = kmalloc(sizeof(*cd), GFP_KERNEL);
	if (!cd)
		return NULL;
	cd->count[0] = 0;
	cd->count[1] = 0;
	cd->sort_list[0] = RB_ROOT;
	cd->sort_list[1] = RB_ROOT;
	cd->curr = 0;
	cd->last_sector = 0;
	return cd;
}

static struct elevator_type cscan = {
	.ops = {
		// .elevator_merge_fn = cscan_merge,
		.elevator_merge_req_fn = cscan_merged_requests,
		// .elevator_merged_fn = cscan_merged_request,
		.elevator_add_req_fn = cscan_add_request,
		.elevator_queue_empty_fn =	cscan_queue_empty,
		.elevator_dispatch_fn =	cscan_dispatch_requests,
		// .elevator_former_req_fn = cscan_former_request,
		// .elevator_latter_req_fn = cscan_latter_request,
		.elevator_set_req_fn = cscan_set_request,
		.elevator_put_req_fn = cscan_put_request,
		.elevator_init_fn = cscan_init_queue,
		.elevator_exit_fn = cscan_exit_queue,
	},
	.elevator_name = "cscan",
	.elevator_owner = THIS_MODULE,
};

static int __init cscan_init(void)
{
	int ret;
	__crq_pool = kmem_cache_create("cscan_crq", sizeof(struct cscan_request), 
		0, 0, NULL);
	crq_pool = mempool_create(BLKDEV_MIN_RQ, mempool_alloc_slab, 
		mempool_free_slab, __crq_pool);
	if (!crq_pool)
		return -ENOMEM;
	elv_register(&cscan);
	return 0;
}

static void __exit cscan_exit(void)
{
	kmem_cache_destroy(__crq_pool);
	elv_unregister(&cscan);
}

module_init(cscan_init);
module_exit(cscan_exit);

MODULE_AUTHOR("Liu Shuo");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CSCAN I/O scheduler");
