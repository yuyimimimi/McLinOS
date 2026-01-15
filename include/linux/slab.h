/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Written by Mark Hemment, 1996 (markhe@nextd.demon.co.uk).
 *
 * (C) SGI 2006, Christoph Lameter
 * 	Cleaned up and restructured to ease the addition of alternative
 * 	implementations of SLAB allocators.
 * (C) Linux Foundation 2008-2013
 *      Unified interface for all slab allocators
 */

#ifndef _LINUX_SLAB_H
#define	_LINUX_SLAB_H

#include <linux/cache.h>
#include <linux/overflow.h>
#include <linux/types.h>
#include <linux/raid/pq.h>
#include <linux/gfp_types.h>
#include <linux/numa.h>
#include <linux/reciprocal_div.h>
#include <linux/spinlock.h>
 
enum _slab_flag_bits {
	_SLAB_CONSISTENCY_CHECKS,
	_SLAB_RED_ZONE,
	_SLAB_POISON,
	_SLAB_KMALLOC,
	_SLAB_HWCACHE_ALIGN,
	_SLAB_CACHE_DMA,
	_SLAB_CACHE_DMA32,
	_SLAB_STORE_USER,
	_SLAB_PANIC,
	_SLAB_TYPESAFE_BY_RCU,
	_SLAB_TRACE,
#ifdef CONFIG_DEBUG_OBJECTS
	_SLAB_DEBUG_OBJECTS,
#endif
	_SLAB_NOLEAKTRACE,
	_SLAB_NO_MERGE,
#ifdef CONFIG_FAILSLAB
	_SLAB_FAILSLAB,
#endif
#ifdef CONFIG_MEMCG
	_SLAB_ACCOUNT,
#endif
#ifdef CONFIG_KASAN_GENERIC
	_SLAB_KASAN,
#endif
	_SLAB_NO_USER_FLAGS,
#ifdef CONFIG_KFENCE
	_SLAB_SKIP_KFENCE,
#endif
#ifndef CONFIG_SLUB_TINY
	_SLAB_RECLAIM_ACCOUNT,
#endif
	_SLAB_OBJECT_POISON,
	_SLAB_CMPXCHG_DOUBLE,
#ifdef CONFIG_SLAB_OBJ_EXT
	_SLAB_NO_OBJ_EXT,
#endif
	_SLAB_FLAGS_LAST_BIT
};



#define __SLAB_FLAG_BIT(nr)	((slab_flags_t __force)(1U << (nr)))
#define __SLAB_FLAG_UNUSED	((slab_flags_t __force)(0U))

/*
 * Flags to pass to kmem_cache_create().
 * The ones marked DEBUG need CONFIG_SLUB_DEBUG enabled, otherwise are no-op
 */
/* DEBUG: Perform (expensive) checks on alloc/free */
#define SLAB_CONSISTENCY_CHECKS	__SLAB_FLAG_BIT(_SLAB_CONSISTENCY_CHECKS)
/* DEBUG: Red zone objs in a cache */
#define SLAB_RED_ZONE		__SLAB_FLAG_BIT(_SLAB_RED_ZONE)
/* DEBUG: Poison objects */
#define SLAB_POISON		__SLAB_FLAG_BIT(_SLAB_POISON)
/* Indicate a kmalloc slab */
#define SLAB_KMALLOC		__SLAB_FLAG_BIT(_SLAB_KMALLOC)
/**
 * define SLAB_HWCACHE_ALIGN - Align objects on cache line boundaries.
 *
 * Sufficiently large objects are aligned on cache line boundary. For object
 * size smaller than a half of cache line size, the alignment is on the half of
 * cache line size. In general, if object size is smaller than 1/2^n of cache
 * line size, the alignment is adjusted to 1/2^n.
 *
 * If explicit alignment is also requested by the respective
 * &struct kmem_cache_args field, the greater of both is alignments is applied.
 */
#define SLAB_HWCACHE_ALIGN	__SLAB_FLAG_BIT(_SLAB_HWCACHE_ALIGN)
/* Use GFP_DMA memory */
#define SLAB_CACHE_DMA		__SLAB_FLAG_BIT(_SLAB_CACHE_DMA)
/* Use GFP_DMA32 memory */
#define SLAB_CACHE_DMA32	__SLAB_FLAG_BIT(_SLAB_CACHE_DMA32)
/* DEBUG: Store the last owner for bug hunting */
#define SLAB_STORE_USER		__SLAB_FLAG_BIT(_SLAB_STORE_USER)
/* Panic if kmem_cache_create() fails */
#define SLAB_PANIC		__SLAB_FLAG_BIT(_SLAB_PANIC)
/**
 * define SLAB_TYPESAFE_BY_RCU - **WARNING** READ THIS!
 *
 * This delays freeing the SLAB page by a grace period, it does _NOT_
 * delay object freeing. This means that if you do kmem_cache_free()
 * that memory location is free to be reused at any time. Thus it may
 * be possible to see another object there in the same RCU grace period.
 *
 * This feature only ensures the memory location backing the object
 * stays valid, the trick to using this is relying on an independent
 * object validation pass. Something like:
 *
 * ::
 *
 *  begin:
 *   rcu_read_lock();
 *   obj = lockless_lookup(key);
 *   if (obj) {
 *     if (!try_get_ref(obj)) // might fail for free objects
 *       rcu_read_unlock();
 *       goto begin;
 *
 *     if (obj->key != key) { // not the object we expected
 *       put_ref(obj);
 *       rcu_read_unlock();
 *       goto begin;
 *     }
 *   }
 *  rcu_read_unlock();
 *
 * This is useful if we need to approach a kernel structure obliquely,
 * from its address obtained without the usual locking. We can lock
 * the structure to stabilize it and check it's still at the given address,
 * only if we can be sure that the memory has not been meanwhile reused
 * for some other kind of object (which our subsystem's lock might corrupt).
 *
 * rcu_read_lock before reading the address, then rcu_read_unlock after
 * taking the spinlock within the structure expected at that address.
 *
 * Note that it is not possible to acquire a lock within a structure
 * allocated with SLAB_TYPESAFE_BY_RCU without first acquiring a reference
 * as described above.  The reason is that SLAB_TYPESAFE_BY_RCU pages
 * are not zeroed before being given to the slab, which means that any
 * locks must be initialized after each and every kmem_struct_alloc().
 * Alternatively, make the ctor passed to kmem_cache_create() initialize
 * the locks at page-allocation time, as is done in __i915_request_ctor(),
 * sighand_ctor(), and anon_vma_ctor().  Such a ctor permits readers
 * to safely acquire those ctor-initialized locks under rcu_read_lock()
 * protection.
 *
 * Note that SLAB_TYPESAFE_BY_RCU was originally named SLAB_DESTROY_BY_RCU.
 */
#define SLAB_TYPESAFE_BY_RCU	__SLAB_FLAG_BIT(_SLAB_TYPESAFE_BY_RCU)
/* Trace allocations and frees */
#define SLAB_TRACE		__SLAB_FLAG_BIT(_SLAB_TRACE)

/* Flag to prevent checks on free */
#ifdef CONFIG_DEBUG_OBJECTS
# define SLAB_DEBUG_OBJECTS	__SLAB_FLAG_BIT(_SLAB_DEBUG_OBJECTS)
#else
# define SLAB_DEBUG_OBJECTS	__SLAB_FLAG_UNUSED
#endif

/* Avoid kmemleak tracing */
#define SLAB_NOLEAKTRACE	__SLAB_FLAG_BIT(_SLAB_NOLEAKTRACE)

/*
 * Prevent merging with compatible kmem caches. This flag should be used
 * cautiously. Valid use cases:
 *
 * - caches created for self-tests (e.g. kunit)
 * - general caches created and used by a subsystem, only when a
 *   (subsystem-specific) debug option is enabled
 * - performance critical caches, should be very rare and consulted with slab
 *   maintainers, and not used together with CONFIG_SLUB_TINY
 */
#define SLAB_NO_MERGE		__SLAB_FLAG_BIT(_SLAB_NO_MERGE)

/* Fault injection mark */
#ifdef CONFIG_FAILSLAB
# define SLAB_FAILSLAB		__SLAB_FLAG_BIT(_SLAB_FAILSLAB)
#else
# define SLAB_FAILSLAB		__SLAB_FLAG_UNUSED
#endif
/**
 * define SLAB_ACCOUNT - Account allocations to memcg.
 *
 * All object allocations from this cache will be memcg accounted, regardless of
 * __GFP_ACCOUNT being or not being passed to individual allocations.
 */
#ifdef CONFIG_MEMCG
# define SLAB_ACCOUNT		__SLAB_FLAG_BIT(_SLAB_ACCOUNT)
#else
# define SLAB_ACCOUNT		__SLAB_FLAG_UNUSED
#endif

#ifdef CONFIG_KASAN_GENERIC
#define SLAB_KASAN		__SLAB_FLAG_BIT(_SLAB_KASAN)
#else
#define SLAB_KASAN		__SLAB_FLAG_UNUSED
#endif

/*
 * Ignore user specified debugging flags.
 * Intended for caches created for self-tests so they have only flags
 * specified in the code and other flags are ignored.
 */
#define SLAB_NO_USER_FLAGS	__SLAB_FLAG_BIT(_SLAB_NO_USER_FLAGS)

#ifdef CONFIG_KFENCE
#define SLAB_SKIP_KFENCE	__SLAB_FLAG_BIT(_SLAB_SKIP_KFENCE)
#else
#define SLAB_SKIP_KFENCE	__SLAB_FLAG_UNUSED
#endif

/* The following flags affect the page allocator grouping pages by mobility */
/**
 * define SLAB_RECLAIM_ACCOUNT - Objects are reclaimable.
 *
 * Use this flag for caches that have an associated shrinker. As a result, slab
 * pages are allocated with __GFP_RECLAIMABLE, which affects grouping pages by
 * mobility, and are accounted in SReclaimable counter in /proc/meminfo
 */
#ifndef CONFIG_SLUB_TINY
#define SLAB_RECLAIM_ACCOUNT	__SLAB_FLAG_BIT(_SLAB_RECLAIM_ACCOUNT)
#else
#define SLAB_RECLAIM_ACCOUNT	__SLAB_FLAG_UNUSED
#endif
#define SLAB_TEMPORARY		SLAB_RECLAIM_ACCOUNT	/* Objects are short-lived */

/* Slab created using create_boot_cache */
#ifdef CONFIG_SLAB_OBJ_EXT
#define SLAB_NO_OBJ_EXT		__SLAB_FLAG_BIT(_SLAB_NO_OBJ_EXT)
#else
#define SLAB_NO_OBJ_EXT		__SLAB_FLAG_UNUSED
#endif

/*
 * freeptr_t represents a SLUB freelist pointer, which might be encoded
 * and not dereferenceable if CONFIG_SLAB_FREELIST_HARDENED is enabled.
 */
typedef struct { unsigned long v; } freeptr_t;

/*
 * ZERO_SIZE_PTR will be returned for zero sized kmalloc requests.
 *
 * Dereferencing ZERO_SIZE_PTR will lead to a distinct access fault.
 *
 * ZERO_SIZE_PTR can be passed to kfree though in the same way that NULL can.
 * Both make kfree a no-op.
 */
#define ZERO_SIZE_PTR ((void *)16)

#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= \
				(unsigned long)ZERO_SIZE_PTR)





#ifdef CONFIG_SLUB_CPU_PARTIAL
#define slub_percpu_partial(c)			((c)->partial)

#define slub_set_percpu_partial(c, p)		\
({						\
	slub_percpu_partial(c) = (p)->next;	\
})

#define slub_percpu_partial_read_once(c)	READ_ONCE(slub_percpu_partial(c))
#else
#define slub_percpu_partial(c)			NULL

#define slub_set_percpu_partial(c, p)

#define slub_percpu_partial_read_once(c)	NULL


#endif // CONFIG_SLUB_CPU_PARTIAL

/*
	* Word size structure that can be atomically updated or read and that
	* contains both the order and the number of objects that a slab of the
	* given order would contain.
	*/				
struct kmem_cache_order_objects {
	unsigned int x;
};

struct kmem_cache_node {
	spinlock_t list_lock;
	unsigned long nr_partial;
	struct list_head partial;
#ifdef CONFIG_SLUB_DEBUG
	atomic_long_t nr_slabs;
	atomic_long_t total_objects;
	struct list_head full;
#endif
};

struct kmem_cache {
	#ifndef CONFIG_SLUB_TINY
	//	struct kmem_cache_cpu __percpu *cpu_slab;
	#endif
		/* Used for retrieving partial slabs, etc. */
		slab_flags_t flags;
		unsigned long min_partial;
		unsigned int size;		/* Object size including metadata */
		unsigned int object_size;	/* Object size without metadata */
		struct reciprocal_value reciprocal_size;
		unsigned int offset;		/* Free pointer offset */
	#ifdef CONFIG_SLUB_CPU_PARTIAL
		/* Number of per cpu partial objects to keep around */
		unsigned int cpu_partial;
		/* Number of per cpu partial slabs to keep around */
		unsigned int cpu_partial_slabs;
	#endif
		struct kmem_cache_order_objects oo;
	
		/* Allocation and freeing of slabs */
		struct kmem_cache_order_objects min;
		gfp_t allocflags;		/* gfp flags to use on each alloc */
		int refcount;			/* Refcount for slab cache destroy */
		void (*ctor)(void *object);	/* Object constructor */
		unsigned int inuse;		/* Offset to metadata */
		unsigned int align;		/* Alignment */
		unsigned int red_left_pad;	/* Left redzone padding size */
		const char *name;		/* Name (only for display!) */
		struct list_head list;		/* List of slab caches */
	#ifdef CONFIG_SYSFS
		struct kobject kobj;		/* For sysfs */
	#endif
	#ifdef CONFIG_SLAB_FREELIST_HARDENED
		unsigned long random;
	#endif
	
	#ifdef CONFIG_NUMA
		/*
			* Defragmentation by allocating from a remote node.
			*/
		unsigned int remote_node_defrag_ratio;
	#endif
	
	#ifdef CONFIG_SLAB_FREELIST_RANDOM
		unsigned int *random_seq;
	#endif
	
	#ifdef CONFIG_KASAN_GENERIC
		struct kasan_cache kasan_info;
	#endif
	
	#ifdef CONFIG_HARDENED_USERCOPY
		unsigned int useroffset;	/* Usercopy region offset */
		unsigned int usersize;		/* Usercopy region size */
	#endif
	
		struct kmem_cache_node *node[MAX_NUMNODES];
	};
					




#define KMALLOC_WAIT 1


extern void* __smalloc__(u32 size, gfp_t flags);
extern void  __sfree__(void* addr);


static void inline *vmalloc(unsigned long size){
	return __smalloc__(size,GFP_TRANSHUGE_LIGHT);
}

static void inline vfree(void *addr){
	__sfree__(addr);
}

static void inline *kmalloc(size_t size, gfp_t flags){
	return __smalloc__((u32)size,flags);
}

static void inline kfree(const void *ptr){
	__sfree__((void*)ptr);
}



static inline void *kmalloc_array(size_t n, size_t size, gfp_t flags){
	return kmalloc(n * size, flags);
}


static void kfree_array(void **ptr,int n){
	int i;
	if (ptr) {
		for (i = 0; i < n; i++) {
			__sfree__(ptr[i]);
		}
		__sfree__(ptr);
	}
}

static inline void* kmalloc_node_noprof(size_t size, gfp_t flags, int node)
{
    return kmalloc((u32)size,flags);
}


static void inline *kzalloc(size_t size, gfp_t flags){
	void* data = __smalloc__((u32)size,flags);
	memset(data,0,size);
	return data;
}





#endif	/* _LINUX_SLAB_H */
