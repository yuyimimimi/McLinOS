#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/atomic.h>


struct request *request_alloc(struct request_queue *q, blk_opf_t opf, gfp_t gfp_mask)
{
    struct request *rq = kmalloc(sizeof(struct request), gfp_mask);
    if (!rq)  -ENOMEM;
    memset(rq, 0, sizeof(struct request));

    rq->q = q;

    rq->cmd_flags = opf;
    rq->__data_len = 0;
    rq->__sector = 0;
    rq->bio = rq->biotail = NULL;
    INIT_LIST_HEAD(&rq->queuelist);
    atomic_set(&rq->ref, 1);
    rq->state = MQ_RQ_IDLE;
    
    /* 时间戳初始化 */
    rq->start_time_ns = ktime_get_ns();
    rq->io_start_time_ns = 0;
    
    /* I/O 相关 */
    rq->part = NULL;
    rq->timeout = 1000;
    
    return rq;
}


void __blk_insert_request(struct request *rq, struct bio *bio)
{
    
    bio->bi_next = NULL;

    struct bio *head = rq->bio;
    if(head == NULL)
    {
        rq->bio = bio;
        rq->biotail = bio;
    }
    else
    {
        rq->biotail->bi_next = bio;
        rq->biotail = bio;
    }

    rq->__data_len += 1;
    

}