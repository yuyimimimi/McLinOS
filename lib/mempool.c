#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mempool.h>
#include <linux/slab.h>
#include <linux/export.h>


static void *haper_alloc(size_t size, struct haper2 *pool_data) {
    return fast_alloc(pool_data, size);
}
static void haper_free(void *obj, struct haper2 *pool_data) {
    free_haper_block(pool_data, obj);
}

mempool_t *mempool_create(uint32_t min_nr, size_t obj_size,mempool_alloc_t alloc_fn, mempool_free_t free_fn, void *pool_data) {
    // 分配 mempool_t 结构
    mempool_t *pool = kmalloc(sizeof(mempool_t), GFP_KERNEL);
    if (!pool) return NULL;

    // 分配 haper2 结构
    pool->haper = kmalloc(sizeof(struct haper2), GFP_KERNEL);
    if (!pool->haper) {
    kfree(pool);
    return NULL;
    }

    // 分配 haper_data 缓冲区
    size_t haper_size = min_nr * (obj_size+sizeof(struct memory_block)) ;  // 假设缓冲区大小与对象总数相关

    pool->haper_data = kmalloc(haper_size, GFP_KERNEL);
    if (!pool->haper_data) {
    kfree(pool->haper);
    kfree(pool);
    return NULL;
    }

    // 初始化 haper2
    if (haper2_init(pool->haper, pool->haper_data, haper_size, 0x00) < 0) {
    kfree(pool->haper_data);
    kfree(pool->haper);
    kfree(pool);
    return NULL;
    }

    pool->obj_size = obj_size;
    pool->min_nr = min_nr;
    pool->curr_nr = 0;
    pool->alloc = alloc_fn ? alloc_fn : haper_alloc;  // 默认使用 best_fit_alloc
    pool->free = free_fn ? free_fn : haper_free;      // 默认使用 free_haper_block
    pool->pool_data = pool->haper;  // 将 haper 作为 pool_data 传递给回调

    // 预分配 min_nr 个对象
    for (uint32_t i = 0; i < min_nr; i++) {
    void *obj = pool->alloc(obj_size, pool->pool_data);
    if (obj) {
    pool->curr_nr++;
    pool->free(obj, pool->pool_data);  // 归还到池中
    }
}

return pool;
}

void mempool_destroy(mempool_t *pool) {
    if (!pool) return;
    kfree(pool->haper_data);  // 释放 haper_data 缓冲区
    kfree(pool->haper);       // 释放 haper2 结构
    kfree(pool);              // 释放 mempool_t 结构
}

void *mempool_alloc(mempool_t *pool, gfp_t flags) {
    if (!pool) return NULL;
    void *obj = pool->alloc(pool->obj_size, pool->pool_data);
    if (obj) pool->curr_nr++;
    return obj;
}
void mempool_free(void *obj, mempool_t *pool) {
    if (!pool || !obj) return;
    pool->free(obj, pool->pool_data);
    if (pool->curr_nr > 0) pool->curr_nr--;
}



