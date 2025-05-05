/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/errno.h>



struct page {
	unsigned long flags;		/* Atomic flags, some possibly
					 * updated asynchronously */
	/*
	 * Five words (20/40 bytes) are available in this union.
	 * WARNING: bit 0 of the first word is used for PageTail(). That
	 * means the other users of this union MUST NOT use the bit to
	 * avoid collision and false-positive PageTail().
	 */
	union {
		struct {	/* Page cache and anonymous pages */
			/**
			 * @lru: Pageout list, eg. active_list protected by
			 * lruvec->lru_lock.  Sometimes used as a generic list
			 * by the page owner.
			 */
			union {
				struct list_head lru;

				/* Or, for the Unevictable "LRU list" slot */
				struct {
					/* Always even, to negate PageTail */
					void *__filler;
					/* Count page's or folio's mlocks */
					unsigned int mlock_count;
				};

				/* Or, free page */
				struct list_head buddy_list;
				struct list_head pcp_list;
			};
			/* See page-flags.h for PAGE_MAPPING_FLAGS */
			struct address_space *mapping;
			union {
				pgoff_t index;		/* Our offset within mapping. */
				unsigned long share;	/* share count for fsdax */
			};
			/**
			 * @private: Mapping-private opaque data.
			 * Usually used for buffer_heads if PagePrivate.
			 * Used for swp_entry_t if swapcache flag set.
			 * Indicates order in the buddy system if PageBuddy.
			 */
			unsigned long private;
		};
		struct {	/* page_pool used by netstack */
			/**
			 * @pp_magic: magic value to avoid recycling non
			 * page_pool allocated pages.
			 */
			unsigned long pp_magic;
		//	struct page_pool *pp; 
			unsigned long _pp_mapping_pad;
			unsigned long dma_addr;
		//	atomic_long_t pp_ref_count;
		};
		struct {	/* Tail pages of compound page */
			unsigned long compound_head;	/* Bit zero is set */
		};
		struct {	/* ZONE_DEVICE pages */
			/** @pgmap: Points to the hosting device page map. */
			// struct dev_pagemap *pgmap;
			void *zone_device_data;
			/*
			 * ZONE_DEVICE private pages are counted as being
			 * mapped so the next 3 words hold the mapping, index,
			 * and private fields from the source anonymous or
			 * page cache page while the page is migrated to device
			 * private memory.
			 * ZONE_DEVICE MEMORY_DEVICE_FS_DAX pages also
			 * use the mapping, index, and private fields when
			 * pmem backed DAX files are mapped.
			 */
		};

		/** @rcu_head: You can use this to free a page by RCU. */
		struct rcu_head rcu_head;
	};

	union {		/* This union is 4 bytes in size. */
		/*
		 * For head pages of typed folios, the value stored here
		 * allows for determining what this page is used for. The
		 * tail pages of typed folios will not store a type
		 * (page_type == _mapcount == -1).
		 *
		 * See page-flags.h for a list of page types which are currently
		 * stored here.
		 *
		 * Owners of typed folios may reuse the lower 16 bit of the
		 * head page page_type field after setting the page type,
		 * but must reset these 16 bit to -1 before clearing the
		 * page type.
		 */
		unsigned int page_type;

		/*
		 * For pages that are part of non-typed folios for which mappings
		 * are tracked via the RMAP, encodes the number of times this page
		 * is directly referenced by a page table.
		 *
		 * Note that the mapcount is always initialized to -1, so that
		 * transitions both from it and to it can be tracked, using
		 * atomic_inc_and_test() and atomic_add_negative(-1).
		 */
		atomic_t _mapcount;
	};

	/* Usage count. *DO NOT USE DIRECTLY*. See page_ref.h */
	atomic_t _refcount;

#ifdef CONFIG_MEMCG
	unsigned long memcg_data;
#elif defined(CONFIG_SLAB_OBJ_EXT)
	unsigned long _unused_slab_obj_exts;
#endif

	/*
	 * On machines where all RAM is mapped into kernel address space,
	 * we can simply calculate the virtual address. On machines with
	 * highmem some memory is mapped into kernel virtual memory
	 * dynamically, so we need a place to store that address.
	 * Note that this field could be 16 bits on x86 ... ;)
	 *
	 * Architectures with slow multiplication can define
	 * WANT_PAGE_VIRTUAL in asm/page.h
	 */
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			/* Kernel virtual address (NULL if
					   not kmapped, ie. highmem) */
#endif /* WANT_PAGE_VIRTUAL */

#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
	int _last_cpupid;
#endif

#ifdef CONFIG_KMSAN
	/*
	 * KMSAN metadata for this page:
	 *  - shadow page: every bit indicates whether the corresponding
	 *    bit of the original page is initialized (0) or not (1);
	 *  - origin page: every 4 bytes contain an id of the stack trace
	 *    where the uninitialized value was created.
	 */
	struct page *kmsan_shadow;
	struct page *kmsan_origin;
#endif
};

#if CONFIG_MMU 
  #include <mm_page.h>

#else




#define PG_locked       (1 << 0)
#define PG_referenced   (1 << 1)
#define PG_uptodate     (1 << 2)
#define PG_dirty        (1 << 11)
#define PG_active       (1 << 12)

static inline void __set_page_flag(struct page *page, unsigned long flag){
    if (page)
        page->flags |= flag;
}
static inline void __clear_page_flag(struct page *page, unsigned long flag){
    if (page)
        page->flags &= ~flag;
}

static inline int __test_page_flag(struct page *page, unsigned long flag){
    return (page && (page->flags & flag)) ? 1 : 0;
}

static inline struct page *__page_create(gfp_t flags ){
    struct page *page = (struct page *)kmalloc(sizeof(struct page), flags);
    if (page == NULL)
        return NULL;
    page->flags = 0;
    atomic_set(&page->_mapcount, 0);
    atomic_set(&page->_refcount, 0); // 初始引用计数为1
    page->zone_device_data = NULL;
    return page;
}
static inline void __page_destroy(struct page *page){
    if (page) 
	{
        kfree(page);
		page = NULL;
    }
}


static inline void *__page_address(struct page *page){
    return page ? page->zone_device_data : NULL; 
}
static inline void __set_page_dirty(struct page *page){
    if (page)
	__set_page_flag(page, PG_dirty);
}

static inline void __clear_page_dirty(struct page *page){
    if (page)
	__clear_page_flag(page, PG_dirty);
}

static inline struct page *__alloc_pages( gfp_t flags , unsigned int order ){
	struct page *page = __page_create(flags);
	if(page == NULL)
		return -ENOMEM ;
	void *data = (void *)kmalloc(PAGE_SIZE, flags);
	if(data == NULL){
		__page_destroy(page);
		return -ENOMEM ;
	}
	page->zone_device_data = data;
	return page;
}

static inline void __free_pages(struct page *page, unsigned int order){
    if (page) {
        __page_destroy(page);
    }
}

static inline struct page *__pfn_to_page(unsigned long pfn){
	struct page *page = __page_create(GFP_KERNEL);
	if(page == NULL)
		return -ENOMEM ;
	page->zone_device_data = (void *)(pfn << 0 );
	return page;
}


static inline void __get_page(struct page *page){
    if (page)
        atomic_inc(&page->_refcount);
}


static inline void __put_page(struct page *page){
    if (page && atomic_dec_and_test(&page->_refcount)) {
        __free_pages(page, 0);
    }
}

static void* __kmap(struct page *page){
	if (!page)
		return NULL;
	__get_page(page);
	return page->zone_device_data;
}

static void __kunmap(struct page *page){
	if (page)
	__put_page(page);
}


#define __pa(addr)             (addr - 0 )
#define offset_in_page(addr)   (0)
#define alloc_page(flags)		__alloc_pages(flags, 0)
#define SetPageLocked(page)		__set_page_flag(page, PG_locked)
#define SetPageDirty(page)		__set_page_dirty(page)
#define ClearPageDirty(page)	__clear_page_dirty(page)
#define PageDirty(page)			__test_page_flag(page, PG_dirty)
#define get_page(page)			__get_page(page)
#define put_page(page)			__put_page(page)
#define kmap(page)			    __kmap(page)
#define kunmap(page)			__kunmap(page)
#define pfn_to_page(pfn)		__pfn_to_page(pfn)
#define alloc_pages(flags, order)	__alloc_pages(flags, order)
#define free_pages(page, order)	__free_pages(page, order)
#define virt_to_page(addr)		pfn_to_page(__pa((uint32_t)addr) >> 0 )


#endif







/*
 * A swap entry has to fit into a "unsigned long", as the entry is hidden
 * in the "index" field of the swapper address space.
 */
typedef struct {
	unsigned long val;
} swp_entry_t;

/**
 * struct folio - Represents a contiguous set of bytes.
 * @flags: Identical to the page flags.
 * @lru: Least Recently Used list; tracks how recently this folio was used.
 * @mlock_count: Number of times this folio has been pinned by mlock().
 * @mapping: The file this page belongs to, or refers to the anon_vma for
 *    anonymous memory.
 * @index: Offset within the file, in units of pages.  For anonymous memory,
 *    this is the index from the beginning of the mmap.
 * @private: Filesystem per-folio data (see folio_attach_private()).
 * @swap: Used for swp_entry_t if folio_test_swapcache().
 * @_mapcount: Do not access this member directly.  Use folio_mapcount() to
 *    find out how many times this folio is mapped by userspace.
 * @_refcount: Do not access this member directly.  Use folio_ref_count()
 *    to find how many references there are to this folio.
 * @memcg_data: Memory Control Group data.
 * @virtual: Virtual address in the kernel direct map.
 * @_last_cpupid: IDs of last CPU and last process that accessed the folio.
 * @_entire_mapcount: Do not use directly, call folio_entire_mapcount().
 * @_large_mapcount: Do not use directly, call folio_mapcount().
 * @_nr_pages_mapped: Do not use outside of rmap and debug code.
 * @_pincount: Do not use directly, call folio_maybe_dma_pinned().
 * @_folio_nr_pages: Do not use directly, call folio_nr_pages().
 * @_hugetlb_subpool: Do not use directly, use accessor in hugetlb.h.
 * @_hugetlb_cgroup: Do not use directly, use accessor in hugetlb_cgroup.h.
 * @_hugetlb_cgroup_rsvd: Do not use directly, use accessor in hugetlb_cgroup.h.
 * @_hugetlb_hwpoison: Do not use directly, call raw_hwp_list_head().
 * @_deferred_list: Folios to be split under memory pressure.
 * @_unused_slab_obj_exts: Placeholder to match obj_exts in struct slab.
 *
 * A folio is a physically, virtually and logically contiguous set
 * of bytes.  It is a power-of-two in size, and it is aligned to that
 * same power-of-two.  It is at least as large as %PAGE_SIZE.  If it is
 * in the page cache, it is at a file offset which is a multiple of that
 * power-of-two.  It may be mapped into userspace at an address which is
 * at an arbitrary page offset, but its kernel virtual address is aligned
 * to its size.
 */
struct folio {
	/* private: don't document the anon union */
	union {
		struct {
	/* public: */
			unsigned long flags;
			union {
				struct list_head lru;
	/* private: avoid cluttering the output */
				struct {
					void *__filler;
	/* public: */
					unsigned int mlock_count;
	/* private: */
				};
	/* public: */
			};
			struct address_space *mapping;
			pgoff_t index;
			union {
				void *private;
				swp_entry_t swap;
			};
			atomic_t _mapcount;
			atomic_t _refcount;
#ifdef CONFIG_MEMCG
			unsigned long memcg_data;
#elif defined(CONFIG_SLAB_OBJ_EXT)
			unsigned long _unused_slab_obj_exts;
#endif
#if defined(WANT_PAGE_VIRTUAL)
			void *virtual;
#endif
#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
			int _last_cpupid;
#endif
	/* private: the union with struct page is transitional */
		};
		struct page page;
	};
	union {
		struct {
			unsigned long _flags_1;
			unsigned long _head_1;
	/* public: */
			atomic_t _large_mapcount;
			atomic_t _entire_mapcount;
			atomic_t _nr_pages_mapped;
			atomic_t _pincount;
#ifdef CONFIG_64BIT
			unsigned int _folio_nr_pages;
#endif
	/* private: the union with struct page is transitional */
		};
		struct page __page_1;
	};
	union {
		struct {
			unsigned long _flags_2;
			unsigned long _head_2;
	/* public: */
			void *_hugetlb_subpool;
			void *_hugetlb_cgroup;
			void *_hugetlb_cgroup_rsvd;
			void *_hugetlb_hwpoison;
	/* private: the union with struct page is transitional */
		};
		struct {
			unsigned long _flags_2a;
			unsigned long _head_2a;
	/* public: */
			struct list_head _deferred_list;
	/* private: the union with struct page is transitional */
		};
		struct page __page_2;
	};
};



#endif /* _LINUX_MM_TYPES_H */