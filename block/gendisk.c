#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/sprintf.h>

static char number = 'a';
static uint8_t blk_dev_major = 254;
struct gendisk *__gendisk_alloc(int major,int minors)
{
    struct gendisk *disk;
    disk = kmalloc(sizeof(struct gendisk), GFP_KERNEL);
    if (!disk) return -ENOMEM;
    
    if(major == 0){
        blk_dev_major++;   
        disk->major = blk_dev_major;
    }

    disk->major = major;
    disk->first_minor = 0;
    disk->minors = minors;

    snprintf(disk->disk_name, DISK_NAME_LEN, "sd%c",number);
    number++;
    disk->flags = 0;
    disk->state &= GD_NEED_PART_SCAN;
    disk->state &= GD_NATIVE_CAPACITY;
    mutex_init(&disk->open_mutex);

    disk->open_partitions = 0;
    disk->diskseq = 0;
    disk->open_mode = BLK_OPEN_READ;
    return disk;
}

void __put_disk(struct gendisk *disk){
    kfree(disk);   
}