#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/blkdev.h>
#include <linux/bio.h>  
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>

#define BIO_INLINE_VECS 4

struct bio_set fs_bio_set;

struct bio *bio_alloc_bioset(struct block_device *bdev, unsigned short nr_iovecs,
    blk_opf_t opf, gfp_t gfp_mask,
    struct bio_set *bs)
{
    struct bio *bio;
    size_t bio_size;
    bool use_inline_vecs = false;
    if (nr_iovecs <= BIO_INLINE_VECS)
    {
        use_inline_vecs = true;
        bio_size = sizeof(struct bio) + sizeof(struct bio_vec) * nr_iovecs;
    } 
    else bio_size = sizeof(struct bio);  
    bio = kmalloc(bio_size, gfp_mask);
    if (!bio) return NULL;
    memset(bio, 0, bio_size);
    if (use_inline_vecs) 
    {
        bio->bi_io_vec   = bio->bi_inline_vecs;
        bio->bi_max_vecs = nr_iovecs;
    }
    else 
    {
        bio->bi_io_vec = kmalloc_array(nr_iovecs, sizeof(struct bio_vec), gfp_mask);
        if (!bio->bi_io_vec) {
            kfree(bio);
            return NULL;
        }
        bio->bi_max_vecs = nr_iovecs;
    }
    bio->bi_next   = NULL;
    bio->bi_bdev   = NULL;
    bio->bi_opf    = 0;
    bio->bi_flags  = 0;
    bio->bi_status = BLK_STS_OK;
    atomic_set(&bio->__bi_remaining, 1);
    atomic_set(&bio->__bi_cnt, 1);
    bio->bi_iter.bi_sector = 0;
    bio->bi_iter.bi_size   = 0;
    bio->bi_iter.bi_idx    = 0;
    bio->bi_iter.bi_bvec_done = 0;
    bio->bi_vcnt    = 0;
    bio->bi_end_io  = NULL;
    bio->bi_private = NULL;
    bio->bi_bdev = bdev;
    bio->bi_opf  = opf;
    return bio;
}
EXPORT_SYMBOL(bio_alloc_bioset);
 


void bio_put(struct bio *bio)
{
    if(!bio)return;
    if (atomic_dec_and_test(&bio->__bi_cnt)) {
        if (bio->bi_io_vec != bio->bi_inline_vecs) {
            kfree(bio->bi_io_vec);
        }
        kfree(bio);
    }
    bio = NULL;
}

EXPORT_SYMBOL(bio_put);



int bio_add_page(struct bio *bio, struct page *page,
    unsigned int len, unsigned int offset)
{
    return __bio_add_page(bio,page,len,offset);
}
EXPORT_SYMBOL(bio_add_page);



int __bio_add_page(struct bio *bio, struct page *page,
    unsigned int len, unsigned int off)
{
    struct bio_vec *bv;
    if (unlikely(!bio || !page || off >= PAGE_SIZE)) return  -ENOMEM;;
    if (bio->bi_vcnt >= bio->bi_max_vecs) return -ENOMEM;
    if (bio->bi_iter.bi_size + len > (256 << PAGE_SHIFT))return -ENOMEM;;
    bv = &bio->bi_io_vec[bio->bi_vcnt];
    bv->bv_page           = page;
    bv->bv_offset         = off;
    bv->bv_len            = len;
    bio->bi_iter.bi_size += len;
    bio->bi_vcnt++;
    return 0;
}




void submit_bio_wait(struct bio *bio)
{
    struct request *rq =  blk_get_request(bio->bi_bdev->bd_queue,bio->bi_opf,GFP_KERNEL);  
    rq->__sector = bio->bi_iter.bi_sector + bio->bi_bdev->bd_start_sect;
    blk_submit_bio(rq,bio);   
    spin_lock(&bio->bi_bdev->bd_queue->queue_lock);
    blk_queue_make_request(bio->bi_bdev->bd_queue, rq); 
    spin_unlock(&bio->bi_bdev->bd_queue->queue_lock);
    bio->bi_bdev->bd_queue->q_fn(bio->bi_bdev->bd_queue);
    if(bio->bi_end_io!= NULL)
        bio->bi_end_io(bio);
}
EXPORT_SYMBOL(submit_bio_wait);





static int number;
void submit_bio(struct bio *bio){
    submit_bio_wait(bio);
}
EXPORT_SYMBOL(submit_bio);

void submit_bh(struct bio *bio){
    kthread_run(submit_bio_wait,bio,"_request io thread %d",number);
}
EXPORT_SYMBOL(submit_bh);
