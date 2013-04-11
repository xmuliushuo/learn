// CSCAN磁盘调度算法
// 使用两个队列，按照块号由小到大排列，其中一个为当前队列。
// 当一个磁盘请求到达时，检查该请求的块号是否大于当前队列的最小块号，
// 若大于最小块号，则加入当前队列，否则加入另外一个队列，
// 不停地完成当前队列的请求直到队列为空，然后互换两个队列的角色。
// 刘硕

// +#include <linux/kernel.h>
// +#include <linux/fs.h>
// +#include <linux/blkdev.h>
// +#include <linux/bio.h>
// +#include <linux/config.h>
// +#include <linux/init.h>
// +#include <linux/rbtree.h>
// +#include <linux/hash.h>
// +#include <asm/uaccess.h>
// +
// +
// +#define RB_NONE         (2)
// +#define RB_EMPTY(root)  ((root)->rb_node == NULL)
// +#define ON_RB(node)     ((node)->rb_color != RB_NONE)
// +#define rq_rb_key(rq)   (rq)->sector
// +
// +/*
// + *      Searching/Sorting routines
// + */
// +static struct CSCAN_request *
// +__rb_insert_request(struct rb_root * root, struct CSCAN_request * crq) {
// +        struct rb_node **p = &root->rb_node;
// +        struct rb_node *parent = NULL;
// +        struct CSCAN_request * __crq;
// +        
// +         while(*p) {
// +                parent = *p;
// +                __crq = rb_entry(parent,struct CSCAN_request,rb_node);
// +
// +                if(crq->rb_key < __crq->rb_key) {
// +                        p = &(*p)->rb_left;
// +                } else if(crq->rb_key > __crq->rb_key) {
// +                        p = &(*p)->rb_right;
// +                } else {
// +                        return __crq;
// +                }
// +        }
// +        rb_link_node(&crq->rb_node,parent,p);
// +        return NULL;
// +} 
// +
// +static inline struct
// +CSCAN_request * rb_insert_request(struct rb_root * root, 
// +                                     struct CSCAN_request * crq)
// +{
// +        struct CSCAN_request * ret;
// +        if((ret = __rb_insert_request(root,crq)))
// +                goto out;
// +        rb_insert_color(&crq->rb_node,root);
// +out:
// +        return ret;
// +}
// +
// +static inline struct
// +CSCAN_request * rb_find_request(struct rb_root * root, long key) {
// +        struct rb_node * n = root->rb_node;
// +        struct CSCAN_request * crq;
// +
// +        while(n) 
// +        {
// +                crq = rb_entry(n,struct CSCAN_request,rb_node);
// +
// +                if (key > crq->rb_key) {
// +                        n = n->rb_right;
// +                } else if (key < crq->rb_key) {
// +                        n = n->rb_left;
// +                } else {
// +                        return crq;
// +                }
// +        }
// +        return NULL;
// +}
// +
// +static struct request *
// +cscan_find_crq_rb(struct CSCAN_data * cd, sector_t sector) {
// +        struct CSCAN_request * crq;
// +        int i = 0;
// + 
// +        for(i=0;i<2;i++) {
// +                crq = rb_find_request(&cd->sort_list[i],sector);
// +                if(crq)
// +                        return crq->request;
// +        }
// +
// +        return NULL;
// +}
// +
// +static struct request * 
// +cscan_find_rq_hash(struct CSCAN_data *cd, sector_t sector) {
// +        struct list_head *hash_list = &cd->hash[DL_HASH_FN(sector)];
// +        struct list_head *entry, *next = hash_list->next;
// +
// +        while ((entry = next) != hash_list) {
// +                struct CSCAN_request *crq = list_entry_hash(entry);
// +                struct request *__rq = crq->request;
// +
// +                next = entry->next;
// +
// +                BUG_ON(!ON_HASH(crq));
// +
// +                if (!rq_mergeable(__rq)) {
// +                        cscan_del_crq_hash(crq);
// +                        continue;
// +                }
// +
// +                if (rq_hash_key(__rq) == sector)
// +                        return __rq;
// +        }
// +
// +        return NULL;
// +
// +
// +}
#include <linux/elevator.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/compiler.h>

struct cscan_rq {
	struct request* request;   
	unsigned char queue_id; /* Which queue is this request on */
};

struct cscan_data {
	struct list_head sort_list[2];
	unsigned int curr;
	unsigned int count[2];
	struct list_head * dispatch;    
	mempool_t *crq_pool;
	sector_t last_sector;
};

static kmem_cache_t* crq_pool;

static inline void cscan_del_crq(struct cscan_data *cd, struct cscan_request *crq)
{
	list_del_init(&crq->sort_list[crq->queue_id]);
	cd->count[crq->queue_id]--;
}

static inline void cscan_add_crq(struct cscan_data *cd, struct cscan_request *crq)
{
// +        rb_insert_request(&cd->sort_list[crq->queue_id],crq);
	// TODO 把请求插入到队列中
	cd->count[crq->queue_id]++;
}

// 把crq对应的request添加到调度队列，并且删除之
static void cscan_remove_request(struct cscan_data *cd, struct cscan_request *crq, int queue) {
	struct request * rq;
	rq = crq->request;
	list_add_tail(&rq->queuelist, cd->dispatch);
	cscan_del_crq(cd, crq);
}

static int cscan_move_requests_to_dispatch_queue(struct cscan_data *cd, int queue) {
// +        struct rb_node * node;
// +        struct CSCAN_request * crq;
	int ret = 0;
// +        node = rb_first(&cd->sort_list[queue]);
// +        while(node) {
// +                crq  = rb_entry(node,struct CSCAN_request,rb_node);
// +                node = rb_next(node);
// +                cscan_remove_request(cd,crq,queue); 
// +                ret = 1;
// +        }
// +
	return ret;
}

// 把调度器队列中的请求分发到块设备的请求队列。
static int cscan_dispatch_requests(struct cscab_data *cd) {
	int ret1,ret2;
	ret1 = cscan_move_requests_to_dispatch_queue(cd,cd->curr);
	ret2 = cscan_move_requests_to_dispatch_queue(cd,1 - cd->curr);   
	cd->curr = 1 - cd->curr;
	// Return 1 if you have dispatched requests
	return (ret1 || ret2);      
}

static void cscan_add_request(struct request_queue *q, struct request *rq) {
	struct cscan_data *cd = q->elevator->elevator_data;
	struct cscan_rq *crq = (struct cscan_request*)rq->elevator_private;
	if (rq->sector > crq->last_sector) {
		crq->queue_id = cd->curr;
		cscan_add_crq_rb(cd, crq); 
	}
	else {
		crq->queue_id = 1 - cd->curr;
		cscan_add_crq_rb(cd,crq); 
	}
	if(rq_mergeable(rq)) {
		if(!q->last_merge)
			q->last_merge = rq;    
	}                
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
static int cscan_merge(request_queue_t *q, struct request **req, struct bio *bio)
{
	int ret;

	/*
	 * try last_merge to avoid going to hash
	 */
	ret = elv_try_last_merge(q, bio);
	if (ret != ELEVATOR_NO_MERGE) {
		*req = q->last_merge;
		return ret;
	}

	return ELEVATOR_NO_MERGE;
// +        struct CSCAN_data * cd = q->elevator->elevator_data;
// +        struct request * __rq;
// +        sector_t rb_key;
// +        /*
// +         * see if the merge hash can satisfy a back merge
// +         */
// +        __rq = cscan_find_rq_hash(cd, bio->bi_sector);
// +        if (__rq) {
// +                BUG_ON(__rq->sector + __rq->nr_sectors != bio->bi_sector);
// +
// +                if (elv_rq_merge_ok(__rq, bio)) {
// +                        ret = ELEVATOR_BACK_MERGE;
// +                        goto out;
// +                }
// +        }
// +
// +         /*
// +         * check for front merge
// +         */
// +         rb_key = bio->bi_sector + bio_sectors(bio);
// +
// +         __rq = cscan_find_crq_rb(cd, rb_key);
// +         if (__rq) {
// +                 BUG_ON(rb_key != rq_rb_key(__rq));
// +
// +                 if (elv_rq_merge_ok(__rq, bio)) {
// +                         ret = ELEVATOR_FRONT_MERGE;
// +                         goto out;
// +                 }
// +         }
// +         return ELEVATOR_NO_MERGE;
// +out:
// +        q->last_merge = __rq;
// +
// +out_insert: 
// +        *req = __rq;
// +	return ret;
}

// void elevator_merge_req_fn(struct request_queue *queue, struct request *req1, struct request *req2);
// Description
// Called when two requests have been coalesced into a single request. 
// This occurs when a request grows (as a result of a bio merge) to become adjacent with an existing request.
// Arguments
// req1: This request will grow to accommodate req2.
// req2: This request is coalsced with req1 and will be deallocated.
void cscan_merged_requests(struct request_queue_t *q, struct request *req1, struct request *req2)
{
	// TODO 这里应该删掉req2，并且重新调整req1
	struct cscan_data *cd = q->elevator->elevator_data;
	struct cscan_request *crq = (struct cscan_request *)req->elevator_private;
	struct cscan_request *tnext = (struct cscan_request *)req->elevator_private;
	BUG_ON(!crq);
	BUG_ON(!tnext);
// +        
// +        if(rq_rb_key(req) |= crq->rb_key) {
// +                cscan_del_crq_rb(cd,crq);
// +                cscan_add_crq_rb(cd,crq);
// +        }
// +
// +        cscan_remove_request(cd,tnext,tnext->queue_id);
}

static struct request *cscan_next_request(request_queue_t *q)
{
	struct cscan_data *cd = q->elevator->elevator_data;
	struct request *rq = NULL;        

	if (!list_empty(cd->dispatch)) {
dispatch:
		rq = list_entry_rq(cd->dispatch->next);
		cd->last_sector = rq->sector + rq->nr_sectors;
		return rq;
	}                
	if(cscan_dispatch_requests(cd)) {
		goto dispatch;
	}
	return NULL;
}

// void elevator_add_req_fn(struct request_queue *queue, struct request *req, int where);
// Description
// Queues a new request with the scheduler.
// This function is mandatory.
// Arguments
// req
// The request to be enqueued.
static void cscan_insert_request(request_queue_t *q, struct request *rq, int where)
{
	struct CSCAN_data *cd = q->elevator->elevator_data;

	// 使用likely ，执行if后面语句的可能性大些，编译器将if{}是的内容编译到前面, 
	// 使用unlikely ，执行else后面语句的可能性大些,编译器将else{}里的内容编译到前面。
	// 这样有利于cpu预取,提高预取指令的正确率,因而可提高效率。
	if (unlikely(rq->flags & (REQ_SOFTBARRIER | REQ_HARDBARRIER) && where == ELEVATOR_INSERT_SORT))
		where = ELEVATOR_INSERT_BACK;

	switch (where) {
	case ELEVATOR_INSERT_BACK:
		while (cscan_dispatch_requests(cd))
			;
		list_add_tail(&rq->queuelist, cd->dispatch);
		break;
	case ELEVATOR_INSERT_FRONT:
		list_add(&rq->queuelist, cd->dispatch);
		break;
	case ELEVATOR_INSERT_SORT:
		BUG_ON(!blk_fs_request(rq));
		cscan_add_request(q, rq);
		break;
	default:
		printk("%s: bad insert point %d\n", __FUNCTION__,where);
		return;
	}            
}

static int cscan_queue_empty(request_queue_t *q)
{
	struct cscan_data *cd = q->elevator->elevator_data;
	if (!list_empty(&cd->sort_list[0]) || !list_empty(&cd->sort_list[1]) || !list_empty(cd->dispatch))
		return 0;
	return 1;
}

// int elevator_set_req_fn(struct request_queue *queue, struct request *req, gfp_t gfp);
// Description
// Allocate scheduler-private data for a request. Called when allocating request.
// Returns 0 if allocation succeeds, non-zero otherwise.
// Arguments
// req
// The request being allocated.
// gfp
// Modifier required for any allocations performed.
static int cscan_set_request(request_queue_t *q, struct request *rq, int gfp_mask) 
{
	struct cscan_data *cd = q->elevator->elevator_data;
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
	// 常用标志值有__GPF_WAIT(分配器可以睡眠)，__GFP_IO(分配器可以启动磁盘I/O)，__GFP_FS(分配器可以启动文件系统I/O)等等。
	// 区修饰符表示从哪个区分配内存，内核优先从ZONE_NORMAL开始分配。区修饰符的值只有__GFP_DMA，__GFP_HIGHMEM。
	// 类型修饰符指定所需的行为和区描述符以完成特殊类型的处理。最常用的值为__GFP_KERNEL （__GFP_WAIT | __GFP_IO | __GFP_FS）
	crq = mempool_alloc(cd->crq_pool, gfp_mask);
	if(crq) {
		memset(crq, 0, sizeof(*crq));
		crq->request = rq;
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
static void cscan_put_request(request_queue_t *q, struct request *rq) 
{
	struct cscan_data *cd = q->elevator->elevator_data;
	struct cscan_request *crq = (struct CSCAN_request*)rq->elevator_private;
	if(crq) {
		mempool_free(crq, cd->crq_pool);
		rq->elevator_private = NULL;
	}
}

// void elevator_exit_fn(elevator_t *elv);
// Description
// Free any resources allocated in elevator_init_fn. No requests should be outstanding when called.
// Arguments
// elv
// The elevator instance to be freed.
static void cscan_exit_queue(elevator_t * e) 
{
	struct cscan_data *cd = e->elevator_data;
	mempool_destroy(cd->crq_pool);
	kfree(cd);
}

// int elevator_init_fn(request_queue_t *q, elevator_t *e);
// Description
// Initialise a new I/O scheduler instance. In particular, allocate and initialise any private data.
// Returns a pointer to the elevator instance private data, or NULL if initialisation fails.
// This function is mandatory.
// Arguments
// queue
// The queue this I/O scheduler will be attached to.
static int cscan_init_queue(request_queue_t *q, elevator_t *e) 
{
	struct cscan_data * cd;
	cd = kmalloc(sizeof(*cd), GFP_KERNEL);
	if (!cd)
		return -ENOMEM;
	memset(cd, 0, sizeof(*cd));
	cd->count[0] = 0;
	cd->count[1] = 0;    
	// mempool_t *mempool_create(int min_nr,
	//                            mempool_alloc_t *alloc_fn,
	//                            mempool_free_t *free_fn,
	//                            void *pool_data);
	// min_nr 参数是内存池应当一直保留的最小数量的分配对象*/
	//
	// 实际的分配和释放对象由 alloc_fn 和 free_fn 处理,原型:
	// typedef void *(mempool_alloc_t)(int gfp_mask, void *pool_data);
	// typedef void (mempool_free_t)(void *element, void *pool_data);
	// 给 mempool_create 最后的参数 *pool_data 被传递给 alloc_fn 和 free_fn
	//
	// 你可编写特殊用途的函数来处理 mempool 的内存分配,但通常只需使用 slab 分配器为你处理这个任务：mempool_alloc_slab 和 mempool_free_slab的原型和上述内存池分配原型匹配，并使用 kmem_cache_alloc 和 kmem_cache_free 处理内存的分配和释放。
	//
	// 典型的设置内存池的代码如下:
	// cache = kmem_cache_create(. . .);
	// pool = mempool_create(MY_POOL_MINIMUM,mempool_alloc_slab, mempool_free_slab, cache);  
	cd->crq_pool = mempool_create(BLKDEV_MIN_RQ, mempool_alloc_slab, mempool_free_slab,crq_pool);
	if(!cd->crq_pool) {
		kfree(cd);
		return -ENOMEM;
	}

	cd->dispatch = &q->queue_head;
	cd->curr = 0;    
	INIT_LIST_HEAD(&dd->sort_list[0]);
	INIT_LIST_HEAD(&dd->sort_list[1]);
	e->elevator_data = cd;
	return 0;
}

static struct elevator_type cscan = {
	.ops = {
		.elevator_merge_fn = cscan_merge,
		.elevator_merge_req_fn = cscan_merged_requests,
		.elevator_next_req_fn = cscan_next_request,
		.elevator_add_req_fn = cscan_insert_request,
		.elevator_set_req_fn = cscan_set_request,
		.elevator_put_req_fn = cscan_put_request,
		.elevator_queue_empty_fn =	cscan_queue_empty,
		.elevator_init_fn = cscan_init_queue,
		.elevator_exit_fn = cscan_exit_queue,
	},
	.elevator_name = "cscan",
	.elevator_owner = THIS_MODULE,
};

int cscan_init(void)
{
	int ret;
	// kmem_cache_create - 创建一个 cache.
	// kmem_cache_t *
	// kmem_cache_create (const char *name, size_t size, size_t align,
	// unsigned long flags, void (*ctor)(void*, kmem_cache_t *, unsigned long),
	// void (*dtor)(void*, kmem_cache_t *, unsigned long))
	// 
	// @name: 此cache在/proc/slabinfo中显示的名字
	// @size: 此cache中对象的大小
	// @align: 对象对齐方式
	// @flags: SLAB 标志
	// @ctor: 对象构造函数.
	// @dtor: 对象析构函数.
	//
	// 成功返回cache的指针，失败返回NULL.
	// 此函数不能在中断中调用，但能被中断.
	// @ctor 将在cache建立新页面时候调用
	// @dtor 将在页面被回收时候调用.
	// @name must be valid until the cache is destroyed. This implies that
	// the module calling this has to destroy the cache before getting unloaded.
	// 
	// The flags are
	// %SLAB_POISON - Poison the slab with a known test pattern (a5a5a5a5)
	// to catch references to uninitialised memory.
	// %SLAB_RED_ZONE - Insert `Red' zones around the allocated memory to check
	// for buffer overruns.
	// %SLAB_HWCACHE_ALIGN - 硬件对齐Align the objects in this cache to a hardware
	// cacheline.  This can be beneficial if you're counting cycles as closely
	// as davem.
	crq_pool = kmem_cache_create("cscan_crq", sizeof(struct cscan_crq), 0, 0, NULL, NULL);
	if (!crq_pool)
		return -ENOMEM;
	ret = elv_register(&cscan);
	if (ret)
		kmem_cache_destroy(crq_pool);
	return ret;
}

void cscan_exit(void)
{
	kmem_cache_destroy(crq_pool);
	elv_unregister(&cscan);
}

module_init(cscan_init);
module_exit(cscan_exit);

MODULE_AUTHOR("Liu Shuo");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CSCAN I/O scheduler");
