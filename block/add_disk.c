#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/string.h>

static void register_son_disk(struct block_device *bdev);
int __register_disk(struct block_device *dev,struct gendisk *disk,char *name);

int __add_disk(struct gendisk *disk)
{
    struct block_device *dev;
    if(disk->part0 == NULL){
        dev = kmalloc(sizeof(struct block_device) , GFP_KERNEL);
        if(dev == NULL){
            printk(KERN_INFO "can not alloc more memory\n");
            return -ENOMEM;
        }   
        disk->part0 = dev;     
    }

    dev = disk->part0;
    dev->bd_disk = disk;
    dev->bd_queue = disk->queue;
    dev->bd_stamp = jiffies;
    dev->bd_dev = disk->major;
    spin_lock_init(&dev->bd_size_lock);
    dev->bd_claiming = NULL;
    dev->bd_holder = NULL;
    mutex_init(&dev->bd_holder_lock);
    mutex_init(&dev->bd_fsfreeze_mutex);
    dev->bd_holders = 0;
    __register_disk(dev,disk,NULL);

    register_son_disk(dev);

    return 0;
}

#define mbr_partiton_startaddress 0x01
#define mbr_partiton_size         0x02
#define mbr_partiton_magic        0x03

extern uint32_t get_partiton_data(struct partition* partition,int number,uint32_t flag);
extern struct partition* get_partition_from_device(struct block_device *bdev);
extern int check_empty_MBR_Table(struct partition *partition,int number);

int add_mbr_son_disk(struct block_device *dev,struct partition* partition,uint32_t number)
{
    struct block_device* son_dev = kmalloc(sizeof(struct block_device) , GFP_KERNEL);
    if(son_dev == NULL){
        printk(KERN_INFO "can not alloc more memory\n");
        return -ENOMEM;
    }
    if(check_empty_MBR_Table(partition,number) == 1){
         return -1;
    }
    sector_t startaddress = get_partiton_data(partition,number,mbr_partiton_startaddress);
    sector_t size  = get_partiton_data(partition,number,mbr_partiton_size);
    uint8_t flag = get_partiton_data(partition,number,mbr_partiton_magic);

    son_dev->bd_start_sect = startaddress;
    son_dev->bd_nr_sectors = size;
    son_dev->bd_disk = dev->bd_disk;
    son_dev->bd_stamp = jiffies;
    son_dev->bd_queue = dev->bd_queue;
    son_dev->bd_dev   = dev->bd_dev++;
    spin_lock_init(&son_dev->bd_size_lock);
    son_dev->bd_claiming = NULL;
    son_dev->bd_holder = NULL;
    mutex_init(&son_dev->bd_holder_lock);
    mutex_init(&son_dev->bd_fsfreeze_mutex);
    son_dev->bd_holders = 0;
    char name[32];
    memset(name,"\0",32);
    strcpy(name,dev->bd_disk->disk_name);
    name[strlen(name)+1] =  name[strlen(name)];
    name[strlen(name)] = '0' + number;

    __register_disk(son_dev,dev->bd_disk,name);
    return 0;
}

static void register_son_disk(struct block_device *bdev)
{
    struct partition* p = get_partition_from_device(bdev);
    if(IS_ERR(p)){ 
         printk(KERN_INFO "no mbr partition table\n");
         return;
    }
    printk(KERN_INFO "dected mbr partition table\n");

    for(int i =0;i<4;i++){
        if(add_mbr_son_disk(bdev,p,i) < 0)break;
    }
}



