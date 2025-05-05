#ifndef _LINUX_BIO_H
#define _LINUX_BIO_H

#include <linux/blk_types.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include <linux/workqueue_types.h>

/* struct bio, bio_vec and BIO_* flags are defined in blk_types.h */
#include <linux/uio.h>
#include <linux/bvec.h>
#include <linux/atomic.h>

struct bio ;

#define page_address(x) x

#define BIO_MAX_VECS		256u

struct queue_limits;

static inline unsigned int bio_max_segs(unsigned int nr_segs)
{
	return min(nr_segs, BIO_MAX_VECS);
}

#define bio_prio(bio)			(bio)->bi_ioprio
#define bio_set_prio(bio, prio)		((bio)->bi_ioprio = prio)

#define bio_iter_iovec(bio, iter)				\
	bvec_iter_bvec((bio)->bi_io_vec, (iter))

#define bio_iter_page(bio, iter)				\
	bvec_iter_page((bio)->bi_io_vec, (iter))
#define bio_iter_len(bio, iter)					\
	bvec_iter_len((bio)->bi_io_vec, (iter))
#define bio_iter_offset(bio, iter)				\
	bvec_iter_offset((bio)->bi_io_vec, (iter))


#define bio_page(bio)		bio_iter_page((bio), (bio)->bi_iter)
#define bio_offset(bio)		bio_iter_offset((bio), (bio)->bi_iter)
#define bio_iovec(bio)		bio_iter_iovec((bio), (bio)->bi_iter)

#define bvec_iter_sectors(iter)	((iter).bi_size >> 9)
#define bvec_iter_end_sector(iter) ((iter).bi_sector + bvec_iter_sectors((iter)))

#define bio_sectors(bio)	bvec_iter_sectors((bio)->bi_iter)
#define bio_end_sector(bio)	bvec_iter_end_sector((bio)->bi_iter)

/*
 * Return the data direction, READ or WRITE.
 */
#define bio_data_dir(bio) \
	(op_is_write(bio_op(bio)) ? WRITE : READ)


/*
 * Check whether this bio carries any data or not. A NULL bio is allowed.
 */
static inline bool bio_has_data(struct bio *bio)
{
	if (bio &&
	    bio->bi_iter.bi_size &&
	    bio_op(bio) != REQ_OP_DISCARD &&
	    bio_op(bio) != REQ_OP_SECURE_ERASE &&
	    bio_op(bio) != REQ_OP_WRITE_ZEROES)
		return true;

	return false;
}

static inline bool bio_no_advance_iter(const struct bio *bio)
{
	return bio_op(bio) == REQ_OP_DISCARD ||
	       bio_op(bio) == REQ_OP_SECURE_ERASE ||
	       bio_op(bio) == REQ_OP_WRITE_ZEROES;
}


static inline void * bio_data(struct bio *bio)
{
	#if (MMU == _ENABLE_)
	if (bio_has_data(bio)){
		return page_address(bio_page(bio)) + bio_offset(bio);		
	}
	#else
		struct bio_vec *bv = &bio->bi_io_vec[bio->bi_iter.bi_idx];
		return kmap(bv->bv_page);
	#endif	
	return NULL;
}


	




static inline bool bio_flagged(struct bio *bio, unsigned int bit)
{
	return bio->bi_flags & (1U << bit);
}

static inline void bio_set_flag(struct bio *bio, unsigned int bit)
{
	bio->bi_flags |= (1U << bit);
}

static inline void bio_clear_flag(struct bio *bio, unsigned int bit)
{
	bio->bi_flags &= ~(1U << bit);
}

/**
 * struct folio_iter - State for iterating all folios in a bio.
 * @folio: The current folio we're iterating.  NULL after the last folio.
 * @offset: The byte offset within the current folio.
 * @length: The number of bytes in this iteration (will not cross folio
 *	boundary).
 */
struct folio_iter {
	struct folio *folio;
	size_t offset;
	size_t length;
	/* private: for use by the iterator */
	struct folio *_next;
	size_t _seg_count;
	int _i;
};



void submit_bio(struct bio *bio);
void submit_bio_wait(struct bio *bio);

int __must_check bio_add_page(struct bio *bio, struct page *page,
    unsigned int len, unsigned int offset);

int __bio_add_page(struct bio *bio, struct page *page,
    unsigned int len, unsigned int off);

extern struct bio_set fs_bio_set;

struct bio *bio_alloc_bioset(struct block_device *bdev, unsigned short nr_iovecs,
    blk_opf_t opf, gfp_t gfp_mask,
    struct bio_set *bs);

static inline struct bio *bio_alloc(struct block_device	*bi_bdev,
		 unsigned short nr_iovecs , blk_opf_t opf, gfp_t gfp_mask)
{
	return bio_alloc_bioset(bi_bdev,nr_iovecs,opf,gfp_mask,&fs_bio_set);
}

extern void bio_put(struct bio *);



static inline void bio_associate_blkg(struct bio *bio) { }

static inline void bio_set_dev(struct bio *bio, struct block_device *bdev)
{
	bio_clear_flag(bio, BIO_REMAPPED);
	if (bio->bi_bdev != bdev)
		bio_clear_flag(bio, BIO_BPS_THROTTLED);
	bio->bi_bdev = bdev;
	bio_associate_blkg(bio);
}




/*
 * BIO list management for use by remapping drivers (e.g. DM or MD) and loop.
 *
 * A bio_list anchors a singly-linked list of bios chained through the bi_next
 * member of the bio.  The bio_list also caches the last list member to allow
 * fast access to the tail.
 */
struct bio_list {
	struct bio *head;
	struct bio *tail;
};

static inline int bio_list_empty(const struct bio_list *bl)
{
	return bl->head == NULL;
}

static inline void bio_list_init(struct bio_list *bl)
{
	bl->head = bl->tail = (struct bio *)NULL;
}

#define BIO_EMPTY_LIST	{ NULL, NULL }

#define bio_list_for_each(bio, bl) \
	for (bio = (bl)->head; bio; bio = bio->bi_next)

static inline unsigned bio_list_size(const struct bio_list *bl)
{
	unsigned sz = 0;
	struct bio *bio;

	bio_list_for_each(bio, bl)
		sz++;

	return sz;
}

static inline void bio_list_add(struct bio_list *bl, struct bio *bio)
{
	bio->bi_next = NULL;

	if (bl->tail)
		bl->tail->bi_next = bio;
	else
		bl->head = bio;

	bl->tail = bio;
}

static inline void bio_list_add_head(struct bio_list *bl, struct bio *bio)
{
	bio->bi_next = bl->head;

	bl->head = bio;

	if (!bl->tail)
		bl->tail = bio;
}

static inline void bio_list_merge(struct bio_list *bl, struct bio_list *bl2)
{
	if (!bl2->head)
		return;

	if (bl->tail)
		bl->tail->bi_next = bl2->head;
	else
		bl->head = bl2->head;

	bl->tail = bl2->tail;
}

static inline void bio_list_merge_init(struct bio_list *bl,
		struct bio_list *bl2)
{
	bio_list_merge(bl, bl2);
	bio_list_init(bl2);
}

static inline void bio_list_merge_head(struct bio_list *bl,
				       struct bio_list *bl2)
{
	if (!bl2->head)
		return;

	if (bl->head)
		bl2->tail->bi_next = bl->head;
	else
		bl->tail = bl2->tail;

	bl->head = bl2->head;
}

static inline struct bio *bio_list_peek(struct bio_list *bl)
{
	return bl->head;
}

static inline struct bio *bio_list_pop(struct bio_list *bl)
{
	struct bio *bio = bl->head;

	if (bio) {
		bl->head = bl->head->bi_next;
		if (!bl->head)
			bl->tail = NULL;

		bio->bi_next = NULL;
	}

	return bio;
}

static inline struct bio *bio_list_get(struct bio_list *bl)
{
	struct bio *bio = bl->head;

	bl->head = bl->tail = NULL;

	return bio;
}

/*
 * Increment chain count for the bio. Make sure the CHAIN flag update
 * is visible before the raised count.
 */
static inline void bio_inc_remaining(struct bio *bio)
{
	bio_set_flag(bio, BIO_CHAIN);
	smp_mb__before_atomic();
	atomic_inc(&bio->__bi_remaining);
}

/*
 * bio_set is used to allow other portions of the IO system to
 * allocate their own private memory pools for bio and iovec structures.
 * These memory pools in turn all allocate from the bio_slab
 * and the bvec_slabs[].
 */
#define BIO_POOL_SIZE 2
 

struct bio_alloc_cache {
	struct bio		*free_list;
	struct bio		*free_list_irq;
	unsigned int		nr;
	unsigned int		nr_irq;
};


struct bio_set {
	struct kmem_cache *bio_slab;
	unsigned int front_pad;

	/*
	 * per-cpu bio alloc cache
	 */
	struct bio_alloc_cache __percpu *cache;

	mempool_t bio_pool;
	mempool_t bvec_pool;
#if defined(CONFIG_BLK_DEV_INTEGRITY)
	mempool_t bio_integrity_pool;
	mempool_t bvec_integrity_pool;
#endif

	unsigned int back_pad;
	/*
	 * Deadlock avoidance for stacking block drivers: see comments in
	 * bio_alloc_bioset() for details
	 */
	spinlock_t		rescue_lock;
	struct bio_list		rescue_list;
	//struct work_struct	rescue_work;
	struct workqueue_struct	*rescue_workqueue;

	/*
	 * Hot un-plug notifier for the per-cpu cache, if used
	 */
	//struct hlist_node cpuhp_dead;
};


#endif