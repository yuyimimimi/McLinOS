#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/blkdev.h>
#include <linux/list.h>



struct request_queue *request_queue_init(int id, struct gendisk *gd,gfp_t flags)
{
    struct request_queue *q;

    q = kmalloc(sizeof(*q), GFP_KERNEL);  // 分配内存
    if (!q)
        return NULL;
    q->queuedata = NULL;               // 初始化为 NULL，后期可分配数据
    q->last_merge = NULL;              // 无合并请求
    spin_lock_init(&q->queue_lock);    // 初始化自旋锁
    q->quiesce_depth = 0;              // 初始静止深度为 0
    q->disk = gd;                      // 关联的磁盘设备
    q->limits.max_sectors = 128;       // 假设队列的最大扇区数为 128
    q->limits.max_segment_size = 32;   // 假设最大段数为 32
    q->id = id;                        // 队列的唯一标识符
    q->nr_requests = 1000;             // 队列支持的最大请求数
    INIT_LIST_HEAD(&q->icq_list);      // 初始化链表头
    q->node = 0;                       // 默认 NUMA 节点为 0
    return q;
}
EXPORT_SYMBOL(request_queue_init);

void __blk_cleanup_queue(struct request_queue *q){
    if(q) kfree(q);
}
 
void request_queue_add(struct request_queue *q, struct request *req)
{
    list_add_tail(&req->queuelist, &q->icq_list);  // 将请求添加到队列末尾
}
EXPORT_SYMBOL(request_queue_add);

void request_queue_remove(struct request_queue *q, struct request *req)
{
    spin_lock(&q->queue_lock);
    if (!list_empty(&req->queuelist)) {
        list_del_init(&req->queuelist);  // 确保 `req->queuelist` 变为未使用状态
        vfree(req);
    } 
    spin_unlock(&q->queue_lock);
}
EXPORT_SYMBOL(request_queue_remove);


void process_requests_in_queue(struct request_queue *q)
{
    struct request *req, *tmp;

    spin_lock(&q->queue_lock);

    list_for_each_entry_safe(req, tmp, &q->icq_list, queuelist) {
        list_del(&req->queuelist);
        spin_unlock(&q->queue_lock);

        // 处理请求
        printk(KERN_INFO "Processing request: %p\n", req);
        kfree(req);  // 释放请求

        spin_lock(&q->queue_lock);
    }

    spin_unlock(&q->queue_lock);
}
EXPORT_SYMBOL(process_requests_in_queue);


struct request *blk_fetch_request(struct request_queue *q)
{
    struct request *req = NULL;
    spin_lock(&q->queue_lock);

    if (!list_empty(&q->icq_list)) {
        req = list_first_entry(&q->icq_list, struct request, queuelist);
        list_del(&req->queuelist);
    }
    spin_unlock(&q->queue_lock);
    return req; 
}
EXPORT_SYMBOL(blk_fetch_request);


int __blk_end_request_cur(struct request *req, int error)
{
    struct request_queue *q = req->q;
    spin_lock(&q->queue_lock);
    int is_last = list_is_singular(&q->icq_list) || list_empty(&q->icq_list);
    spin_unlock(&q->queue_lock);
    return is_last;
}