/* SPDX-License-Identifier: GPL-2.0 */
#ifndef BLK_MQ_H
#define BLK_MQ_H

#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/blk_types.h>
#include <linux/list.h> 


enum rq_end_io_ret {
	RQ_END_IO_NONE,
	RQ_END_IO_FREE,
};

typedef enum rq_end_io_ret (rq_end_io_fn)(struct request *, blk_status_t);

#define BLKDEV_MIN_RQ	4
#define BLKDEV_DEFAULT_RQ	128

/*
 * request flags */
typedef __u32 __bitwise req_flags_t;


/* Keep rqf_name[] in sync with the definitions below */
enum {
	/* drive already may have started this one */
	__RQF_STARTED,
	/* request for flush sequence */
	__RQF_FLUSH_SEQ,
	/* merge of different types, fail separately */
	__RQF_MIXED_MERGE,
	/* don't call prep for this one */
	__RQF_DONTPREP,
	/* use hctx->sched_tags */
	__RQF_SCHED_TAGS,
	/* use an I/O scheduler for this request */
	__RQF_USE_SCHED,
	/* vaguely specified driver internal error.  Ignored by block layer */
	__RQF_FAILED,
	/* don't warn about errors */
	__RQF_QUIET,
	/* account into disk and partition IO statistics */
	__RQF_IO_STAT,
	/* runtime pm request */
	__RQF_PM,
	/* on IO scheduler merge hash */
	__RQF_HASHED,
	/* track IO completion time */
	__RQF_STATS,
	/* Look at ->special_vec for the actual data payload instead of the
	   bio chain. */
	__RQF_SPECIAL_PAYLOAD,
	/* request completion needs to be signaled to zone write plugging. */
	__RQF_ZONE_WRITE_PLUGGING,
	/* ->timeout has been called, don't expire again */
	__RQF_TIMED_OUT,
	__RQF_RESV,
	__RQF_BITS
};

#define RQF_STARTED		((__force req_flags_t)(1 << __RQF_STARTED))
#define RQF_FLUSH_SEQ		((__force req_flags_t)(1 << __RQF_FLUSH_SEQ))
#define RQF_MIXED_MERGE		((__force req_flags_t)(1 << __RQF_MIXED_MERGE))
#define RQF_DONTPREP		((__force req_flags_t)(1 << __RQF_DONTPREP))
#define RQF_SCHED_TAGS		((__force req_flags_t)(1 << __RQF_SCHED_TAGS))
#define RQF_USE_SCHED		((__force req_flags_t)(1 << __RQF_USE_SCHED))
#define RQF_FAILED		((__force req_flags_t)(1 << __RQF_FAILED))
#define RQF_QUIET		((__force req_flags_t)(1 << __RQF_QUIET))
#define RQF_IO_STAT		((__force req_flags_t)(1 << __RQF_IO_STAT))
#define RQF_PM			((__force req_flags_t)(1 << __RQF_PM))
#define RQF_HASHED		((__force req_flags_t)(1 << __RQF_HASHED))
#define RQF_STATS		((__force req_flags_t)(1 << __RQF_STATS))
#define RQF_SPECIAL_PAYLOAD	\
			((__force req_flags_t)(1 << __RQF_SPECIAL_PAYLOAD))
#define RQF_ZONE_WRITE_PLUGGING	\
			((__force req_flags_t)(1 << __RQF_ZONE_WRITE_PLUGGING))
#define RQF_TIMED_OUT		((__force req_flags_t)(1 << __RQF_TIMED_OUT))
#define RQF_RESV		((__force req_flags_t)(1 << __RQF_RESV))

/* flags that prevent us from merging requests: */
#define RQF_NOMERGE_FLAGS \
	(RQF_STARTED | RQF_FLUSH_SEQ | RQF_SPECIAL_PAYLOAD)

enum mq_rq_state {
	MQ_RQ_IDLE		= 0,
	MQ_RQ_IN_FLIGHT		= 1,
	MQ_RQ_COMPLETE		= 2,
};

/*
 * Try to put the fields that are referenced together in the same cacheline.
 *
 * If you modify this structure, make sure to update blk_rq_init() and
 * especially blk_mq_rq_ctx_init() to take care of the added fields.
 */


// struct request {
// 	struct request_queue *q;          //指向请求所属的队列。
// 	//struct blk_mq_ctx *mq_ctx;		  //指向多队列模型中的软件上下文
// 	//struct blk_mq_hw_ctx *mq_hctx;      //指向多队列模型中的硬件上下文

// 	blk_opf_t cmd_flags;		/* op and common flags */
// 	req_flags_t rq_flags;         //请求特定的标志。

// 	int tag;                     //请求的标签。
// 	int internal_tag;            //内部使用的标签。

// 	unsigned int timeout;        //请求的超时时间

// 	/* the following two fields are internal, NEVER access directly */
// 	unsigned int __data_len;	//请求的总数据长度
// 	sector_t __sector;		   //请求的当前扇区位置。/* sector cursor */

// 	struct bio *bio;           //请求关联的 bio 链表的头和尾。bio 是块 I/O 的基本单位，一个 request 可以包含多个 bio。bio 指向链表头，biotail 指向尾部。
// 	struct bio *biotail;

// 	union {                              //请求的队列链接。
// 		struct list_head queuelist;
// 		struct request *rq_next;
// 	};

// 	struct block_device *part;           //指向关联的块设备分区。
// #ifdef CONFIG_BLK_RQ_ALLOC_TIME
// 	/* Time that the first bio started allocating this request. */
// 	u64 alloc_time_ns;      //请求分配开始的时间（纳秒）。
// #endif
// 	/* Time that this request was allocated for this IO. */
// 	u64 start_time_ns;        //请求分配完成的时间（纳秒）。
// 	/* Time that I/O was submitted to the device. */
// 	u64 io_start_time_ns;     //I/O 提交到设备的时间（纳秒）。

// #ifdef CONFIG_BLK_WBT
// 	unsigned short wbt_flags;  //（写回节流）标志。
// #endif
// 	/*
// 	 * rq sectors used for blk stats. It has the same value
// 	 * with blk_rq_sectors(rq), except that it never be zeroed
// 	 * by completion.
// 	 */
// 	unsigned short stats_sectors;  //请求的扇区数统计

// 	/*
// 	 * Number of scatter-gather DMA addr+len pairs after
// 	 * physical address coalescing is performed.
// 	 */
// 	//unsigned short nr_phys_segments;   //物理分散-聚集段的数量。表示经过物理地址合并后的 DMA 段数，用于硬件传输。
// 	//unsigned short nr_integrity_segments; //完整性数据的段数。表示与数据完整性（如 T10 DIF/DIX）相关的段数。

// #ifdef CONFIG_BLK_INLINE_ENCRYPTION         //加密上下文和密钥槽。
// 	struct bio_crypt_ctx *crypt_ctx;
// 	struct blk_crypto_keyslot *crypt_keyslot;
// #endif

// 	enum mq_rq_state state;              //多队列请求的状态。
// 	atomic_t ref;                         //请求的引用计数。

// 	unsigned long deadline;				//请求的截止时间。

// 	/*
// 	 * The hash is used inside the scheduler, and killed once the
// 	 * request reaches the dispatch list. The ipi_list is only used
// 	 * to queue the request for softirq completion, which is long
// 	 * after the request has been unhashed (and even removed from
// 	 * the dispatch list).
// 	 */
// 	// union {                           //请求的哈希或中断链表。
// 	// 	struct hlist_node hash;	/* merge hash */
// 	// 	struct llist_node ipi_list;
// 	// };

// 	/*
// 	 * The rb_node is only used inside the io scheduler, requests
// 	 * are pruned when moved to the dispatch queue. special_vec must
// 	 * only be used if RQF_SPECIAL_PAYLOAD is set, and those cannot be
// 	 * insert into an IO scheduler.
// 	 */
// 	// union {                               //调度器红黑树节点或特殊数据。
// 	// 	struct rb_node rb_node;	/* sort/lookup */
// 	// 	struct bio_vec special_vec;
// 	// };

// 	/*
// 	 * Three pointers are available for the IO schedulers, if they need
// 	 * more they have to dynamically allocate it.
// 	 */
// 	// struct {                                //电梯调度器（elevator）相关数据。
// 	// 	struct io_cq		*icq;
// 	// 	void			*priv[2];
// 	// } elv;

// 	struct {                              //刷新请求相关数据。
// 		unsigned int		seq;
// 		rq_end_io_fn		*saved_end_io;
// 	} flush;

// 	u64 fifo_time;                     //请求进入 FIFO 队列的时间。

// 	/*
// 	 * completion callback.
// 	 */
// 	rq_end_io_fn *end_io;               //请求完成回调及其数据。
// 	void *end_io_data;                  //回调函数的私有数据。
// }; 

struct request {
    struct request_queue *q;          // 指向请求所属的队列
    blk_opf_t cmd_flags;              // 操作类型和通用标志
    req_flags_t rq_flags;             // 请求特定的标志
    int tag;                          // 请求的标签
    int internal_tag;                 // 内部使用的标签
    unsigned int timeout;             // 请求的超时时间
    unsigned int __data_len;          // 请求的总数据长度

    sector_t __sector;                // 请求的当前扇区位置
    
	struct bio *bio;                  // bio 链表头
    struct bio *biotail;              // bio 链表尾
    union {                           // 请求的队列链接
        struct list_head queuelist;
        struct request *rq_next;
    };
    struct block_device *part;        // 指向关联的块设备分区
    u64 start_time_ns;                // 请求分配完成的时间（纳秒）
    u64 io_start_time_ns;             // I/O 提交到设备的时间（纳秒）
    unsigned short stats_sectors;     // 请求的扇区数统计
    enum mq_rq_state state;           // 多队列请求的状态
    atomic_t ref;                     // 请求的引用计数
    unsigned long deadline;           // 请求的截止时间
    struct {                          // 刷新请求相关数据
        unsigned int seq;
        rq_end_io_fn *saved_end_io;
    } flush;
    u64 fifo_time;                    // 请求进入 FIFO 队列的时间
    rq_end_io_fn *end_io;             // 请求完成回调
    void *end_io_data;                // 回调函数的私有数据
};

extern struct request *request_alloc(struct request_queue *q, blk_opf_t opf, gfp_t gfp_mask);


static inline enum req_op req_op(const struct request *req)
{
	return req->cmd_flags & REQ_OP_MASK;
}

static inline bool blk_rq_is_passthrough(struct request *rq)
{
	return blk_op_is_passthrough(rq->cmd_flags);
}

static inline unsigned short req_get_ioprio(struct request *req)
{
	if (req->bio)
		return req->bio->bi_ioprio;
	return 0;
}

#define rq_data_dir(rq)		(op_is_write(req_op(rq)) ? WRITE : READ)




static inline sector_t blk_rq_pos(const struct request *rq)
{
	return rq->__sector;
}

static inline unsigned int blk_rq_bytes(const struct request *rq)
{
	return rq->__data_len;
}

static inline int blk_rq_cur_bytes(const struct request *rq)
{
	if (!rq->bio)
		return 0;
	if (!bio_has_data(rq->bio))	/* dataless requests such as discard */
		return rq->bio->bi_iter.bi_size;
	return bio_iovec(rq->bio).bv_len;
}

static inline unsigned int blk_rq_sectors(const struct request *rq)
{
	return blk_rq_bytes(rq) >> SECTOR_SHIFT;
}

static inline unsigned int blk_rq_cur_sectors(const struct request *rq)
{
	return blk_rq_cur_bytes(rq) >> SECTOR_SHIFT;
}

static inline unsigned int blk_rq_stats_sectors(const struct request *rq)
{
	return rq->stats_sectors;
}

/*
 * Some commands like WRITE SAME have a payload or data transfer size which
 * is different from the size of the request.  Any driver that supports such
 * commands using the RQF_SPECIAL_PAYLOAD flag needs to use this helper to
 * calculate the data transfer size.
 */
// static inline unsigned int blk_rq_payload_bytes(struct request *rq)
// {
// 	if (rq->rq_flags & RQF_SPECIAL_PAYLOAD)
// 		return rq->special_vec.bv_len;
// 	return blk_rq_bytes(rq);
// }



struct blk_mq_tags {
	unsigned int nr_tags;
	unsigned int nr_reserved_tags;
	unsigned int active_queues;

	// struct sbitmap_queue bitmap_tags;
	// struct sbitmap_queue breserved_tags;

	struct request **rqs;
	struct request **static_rqs;
	struct list_head page_list;

	/*
	 * used to clear request reference in rqs[] before freeing one
	 * request pool
	 */
	spinlock_t lock;
};



struct blk_mq_tag_set {
//	const struct blk_mq_ops	*ops;
//	struct blk_mq_queue_map	map[HCTX_MAX_TYPES];
	unsigned int		nr_maps;
	unsigned int		nr_hw_queues;
	unsigned int		queue_depth;
	unsigned int		reserved_tags;
	unsigned int		cmd_size;
	int			numa_node;
	unsigned int		timeout;
	unsigned int		flags;
	void			*driver_data;

	struct blk_mq_tags	**tags;

	struct blk_mq_tags	*shared_tags;

	struct mutex		tag_list_lock;
	struct list_head	tag_list;
//	struct srcu_struct	*srcu;
};







#endif /* BLK_MQ_H */