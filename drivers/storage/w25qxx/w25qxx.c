#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/gpio/gpio.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/spinlock.h>

#include "w25qxx.h"
#ifdef  CONFIG_W25QXX_SPI_FLASH

static struct w25qxx_dev *w25qxx1_dev;
static char *w25qxx_buffer;


static int w25qxx_open(struct gendisk *disk, blk_mode_t mode){
    pr_info("w25qxx block device opened\n\r");
    return 0;
}

static void w25qxx_release(struct gendisk *disk, fmode_t mode){
    pr_info("w25qxx block device released\n");
}

static int w25qxx_getgeo(struct block_device * device , struct hd_geometry * geo){
    geo->heads = 0x01; //没有磁头
    geo->start = 0;
    geo->sectors = w25qxx1_dev->size / 512; //扇区数
    geo->cylinders = 0x00; //没有柱面 
    return 0;
}


static void w25qxx_request(struct request *req )
{
    if(req == NULL) return;
    struct bio *bio2;
    struct bio_vec *bvec;
    struct bvec_iter *iter;
    int ret = 0;
    unsigned long start = blk_rq_pos(req);
    rq_for_each_bio(bio2,req){
        bio_for_each_segment(bvec,bio2,iter){
            int data_length = bvec->bv_len;
            char *buffer = kmap(bvec->bv_page) + bvec->bv_offset;   
            if(rq_data_dir(req) == READ)
            {
                pr_info("w25qxx: read page:%d (%d b)\n",start,data_length);
                ret = W25Qxx_device_read_page(w25qxx1_dev,buffer,data_length,start);  
            }
            else if(rq_data_dir(req) == WRITE){
                int page_4k = start/8;
                int page_4k_offset = start%8;
                pr_info("w25qxx: write page:%d (%d b)\n",start,data_length);
                if(page_4k_offset == 0 && data_length == 4096) //如果4k对其则直接写入
                {
                    ret = W25Qxx_device_write_page(w25qxx1_dev,buffer,data_length,page_4k);     
                }
                else 
                {
                    int lost_data = page_4k_offset*512 + data_length -4096;               //计算数据末端偏移量
                    ret = W25Qxx_device_read_page(w25qxx1_dev,w25qxx_buffer,4096,page_4k);//备份扇区信息
                    if (ret < 0)  break;
                    uint32_t cpoy_length = 4096 - page_4k_offset*512;                    //获取去除偏移地址后剩余可用大小
                    if(cpoy_length > data_length)
                    cpoy_length = data_length;                                           //和原数据长度比较计算实际需写入数据长度
                    memcpy(w25qxx_buffer+page_4k_offset*512,buffer,cpoy_length);         //拷贝信息
                    ret = W25Qxx_device_write_page(w25qxx1_dev,w25qxx_buffer,4096,page_4k);  //写入信息   
                    if (ret < 0)  break;
                    if(lost_data > 0){                                                  //如果此前一次无法完成所有数据传输
                        ret = W25Qxx_device_read_page(w25qxx1_dev,w25qxx_buffer,4096,page_4k+1); 
                        if (ret < 0)  break;
                        cpoy_length = data_length - cpoy_length;                        //计算剩余数据长度
                        memcpy(w25qxx_buffer, buffer+(data_length-cpoy_length),cpoy_length);//拷贝到缓存
                        ret = W25Qxx_device_write_page(w25qxx1_dev,w25qxx_buffer,4096,page_4k+1);//写入信息      
                    }
                }
            }
            put_page(bvec->bv_page);
        }
    }
    if (ret < 0)
    blk_mq_end_request(req,BLK_STS_IOERR);
    else 
    blk_mq_end_request(req,BLK_STS_OK);
}


static void w25qxx_request_queue(struct request_queue *q)
{
    struct request *req= blk_fetch_request(q);
    while (req != NULL){
        w25qxx_request(req);
        req = blk_fetch_request(q);
    };

}

struct block_device_operations w25qxx_ops = {
    .owner = THIS_MODULE,
    .open = w25qxx_open,
    .release = w25qxx_release,
    .getgeo = w25qxx_getgeo,
};


#define DEVICE_NAME   "w25qxx_blk"
static struct gendisk       *w25qxx_disk;
static struct request_queue *w25qxx_queue;
static spinlock_t           w25qxx_lock;
static int major = 0;
static u8 *device_data; 


static int __init w25qxx_init(void)
{
    w25qxx_buffer = vmalloc(4096);
    if(w25qxx_buffer == NULL) {
        pr_info(KERN_ERR "Failed to allocate buffer\n");
        return -ENOMEM;
    }
    w25qxx1_dev = new_w25qxx_dev(CONFIG_W25QXX_SPI_FLASH_SPI_DEVICE,CONFIG_W25QXX_SPI_FLASH_CS_PIN);    //初始化w25qxx1设备设置使用spi1 cs引脚为AP4
    if(w25qxx1_dev == NULL){
        pr_err("w25qxx1 device init failed\n\r");
        vfree(w25qxx_buffer);
        return -1;
    }
    w25qxx_queue = blk_init_queue(w25qxx_request_queue,&w25qxx_lock);   //创建request_queue
    if (!w25qxx_queue) {
        vfree(w25qxx_buffer); 
        kfree(w25qxx1_dev);
        return -ENOMEM;
    }
    w25qxx_queue->limits.physical_block_size = 4096;

    w25qxx_disk = alloc_disk(1);                  //创建gendisk
    if (!w25qxx_disk) {
        vfree(w25qxx_buffer);
        kfree(w25qxx1_dev);
        blk_cleanup_queue(w25qxx_queue);
        return -ENOMEM;
    } 
    w25qxx_disk->part0 = kmalloc(sizeof(struct block_device),GFP_KERNEL);
    if( w25qxx_disk->part0 == NULL){
        vfree(w25qxx_buffer);
        kfree(w25qxx1_dev);
        put_disk(w25qxx_disk);
        blk_cleanup_queue(w25qxx_queue);
        return -ENOMEM;
    }
    w25qxx_disk->part0->bd_start_sect = 0;
    printk(KERN_INFO "w25qxx size has: %d block\n",w25qxx1_dev->size/4096);
    w25qxx_disk->part0->bd_nr_sectors = w25qxx1_dev->size/512;

    w25qxx_disk->major = major;
    w25qxx_disk->first_minor = 0;
    w25qxx_disk->fops = &w25qxx_ops;
    w25qxx_disk->queue = w25qxx_queue;

    int ret = add_disk(w25qxx_disk);
    if(ret < 0)
    {
        printk(KERN_ERR "add disk faled\n");
        vfree(w25qxx_buffer);
        blk_cleanup_queue(w25qxx_queue);
        kfree(w25qxx1_dev);
        put_disk(w25qxx_disk);
        return -ENOMEM;
    }

    printk(KERN_INFO "w25qxx block device stack ready!\n");
    return 0;
}

static void __exit w25qxx_exit(void)
{
    if(w25qxx_buffer){
        vfree(w25qxx_buffer);
    }
    if (w25qxx_disk) {
        del_gendisk(w25qxx_disk);
        put_disk(w25qxx_disk);
    }
    if (w25qxx_queue) blk_cleanup_queue(w25qxx_queue);
    if (w25qxx1_dev)  kfree(w25qxx1_dev);
    printk(KERN_INFO "w25qxx block device driver unloaded\n");
}

module_init(w25qxx_init);
module_exit(w25qxx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuyimimimi(github)");
MODULE_DESCRIPTION("W25QXX SPI Flash Block Device Driver");
MODULE_VERSION("1.0");
#endif

