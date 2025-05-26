
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * memory buffer pool support
 */
#ifndef _LINUX_MEMPOOL_H
#define _LINUX_MEMPOOL_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/compiler.h>
#include <linux/numa.h>
#include <linux/mempool_super_haper.h>

typedef void * (mempool_alloc_t)(gfp_t gfp_mask, void *pool_data);
typedef void (mempool_free_t)(void *element, void *pool_data);

typedef struct mempool_s {
    struct haper2 *haper;        // 指向 haper2 内存池
    size_t obj_size;             // 每个对象的大小
    uint32_t min_nr;             // 最小对象数量
    uint32_t curr_nr;            // 当前对象数量
    mempool_alloc_t *alloc;     // 自定义分配函数
    mempool_free_t *free;       // 自定义释放函数
    void *pool_data;             // 传递给回调的私有数据
    block_t *haper_data;         // haper2 的内存池缓冲区
} mempool_t;

extern mempool_t *mempool_create(uint32_t min_nr, size_t obj_size, mempool_alloc_t alloc_fn, mempool_free_t free_fn, void *pool_data);

extern void mempool_destroy(mempool_t *pool);

extern void *mempool_alloc(mempool_t *pool, gfp_t flags);

extern void mempool_free(void *obj, mempool_t *pool);

#endif /* _LINUX_MEMPOOL_H */
