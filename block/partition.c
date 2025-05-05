#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <asm/byteorder.h>


static void my_end_io(struct bio *bio){
    bio_put(bio);
}
static char * trans_data_to_blk_dev(struct block_device *bdev, sector_t block, unsigned size, char *data, int mod){
    struct page *page = virt_to_page((uint32_t)data);  
    struct bio *bio = bio_alloc(bdev,1,mod,GFP_KERNEL);
    bio_set_dev(bio,bdev);                                           //设置目标核扇区号
    bio->bi_iter.bi_sector = block;
    bio->bi_end_io = my_end_io;
    if(bio_add_page(bio, page, size, offset_in_page(data)) < 0 ){
        bio_put(bio);
        return NULL;
    }
    submit_bio_wait(bio);
    return data;
}




static uint8_t gpt_head[8] = {0x45, 0x46, 0x49, 0x20, 0x50, 0x41, 0x52, 0x54};

static int detect_partition_table(struct block_device *device,char *block_buffer, enum block_device_flags_t* flags)     //读取分区表
{
    *flags = BLOCK_DEVICE_FLAG_NOT_INITIALIZED;                           //设置块设备标志位默认为未初始化
    char *data =  trans_data_to_blk_dev(device,0,512,block_buffer,REQ_OP_READ);
    if(data[0x01FE] == 0x55 && data[0x01FF] == 0xAA)
        *flags = BLOCK_DEVICE_FLAG_MBR;                                    //通过LBA 0判断是否为MBR分区表
    data =  trans_data_to_blk_dev(device,1,512,block_buffer,REQ_OP_READ);//读取LBA 1
    if (memcmp(data, gpt_head, 8) == 0)
    {
        if(*flags == BLOCK_DEVICE_FLAG_MBR)
            *flags = BLOCK_DEVICE_FLAG_PROTECTIVE_MBR;
        else
            *flags = BLOCK_DEVICE_FLAG_GPT;
    }
    return 0;
}

#define INVALID_PARTITION 0xFFFFFFFF

static void MBR_partition_table_fix_endian(struct mbr_partition *mbr_header) 
{
     #ifndef BIG_ENDIAN
      mbr_header->start_lba  = le32_to_cpu(mbr_header->start_lba);
      mbr_header->nr_sectors = le32_to_cpu(mbr_header->nr_sectors);
     #endif
}

int check_empty_MBR_Table(struct partition *partition,int number)
{
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    for(int i =0 ;i < sizeof(struct mbr_partition);i++){
        if(((char *)mbr_partition)[i] != 0)
            return 0;
    }
    return 1;
}
static void clear_MBR_Table(struct partition *partition,int number)
{
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    for(int i =0 ;i < sizeof(struct mbr_partition);i++){
        ((char *)mbr_partition)[i] = 0x00;
    }
}
static struct mbr_partition * get_empty_MBR_Table(struct partition *partition,int *number) //number表示分区所在编号，可以通过它索引前后分区
{
    for(int i = 0 ; i <4 ; i++){
        if(check_empty_MBR_Table(partition,i)){
            number[0] = i;
            return &partition->mbr_partition[i];
        }
    }
    return NULL;
}


static void MBR_partition_table_create(struct mbr_partition * mbr_partition,uint8_t type,uint32_t start_address,uint32_t size, uint8_t flag)
{
    printk("creating new mbr partition: boot_ind = %d, start_address = %d, size = %d,sys_ind = %d\n\r",type,start_address,size,flag);

    mbr_partition->boot_ind     = type; 

    mbr_partition->start_head   = 0x20;     //废弃字段采用固定值填充
    mbr_partition->start_sector = 0X21;
    mbr_partition->start_cyl    = 0x00;
    mbr_partition->sys_ind = flag;
    mbr_partition->end_head     = 0xfe;
    mbr_partition->end_sector   = 0x3f;
    mbr_partition->end_cyl      = 0x01;
    mbr_partition->start_lba = start_address;
    mbr_partition->nr_sectors = size;
}

static uint32_t get_mbr_partition_size(struct partition *partition,int number){
    if(number < 0 || number > 3)
        return INVALID_PARTITION;
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    if(check_empty_MBR_Table(partition,number) == 1)
        return INVALID_PARTITION;
    return mbr_partition->nr_sectors;
}

static uint32_t get_mbr_partition_start_address(struct partition *partition,int number){
    if(number < 0 || number > 3)
        return INVALID_PARTITION;
    
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    if(check_empty_MBR_Table(partition,number) == 1){
         printk("This is a empty partition table slot\n\r");
         return INVALID_PARTITION;
    }
    return mbr_partition->start_lba;
}

static uint8_t get_mbr_partition_type(struct partition *partition,int number){
    if(number < 0 || number > 3)
        return 0xff;
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    if(check_empty_MBR_Table(partition,number) == 1)
        return 0xff;
    return mbr_partition->sys_ind;
}

static int set_mbr_partition_size(struct partition *partition,int number,uint32_t size)
{
    if(number < 0 || number > 3)
        return -1;
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    if(check_empty_MBR_Table(partition,number) == 1)
        return -1;
    mbr_partition->nr_sectors = size;
    return 0;
}

static int set_mbr_partition_start_address(struct partition *partition,int number,uint32_t start_address)
{
    if(number < 0 || number > 3)
        return -1;
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    if(check_empty_MBR_Table(partition,number) == 1)
        return -1;
    mbr_partition->start_lba = start_address;
    return 0;
}

static int set_mbr_partition_type(struct partition *partition,int number,uint8_t type)
{
    if(number < 0 || number > 3)
        return -1;
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    if(check_empty_MBR_Table(partition,number) == 1)
        return -1;
    mbr_partition->sys_ind = type;
    return 0;
}

static void move_MBR_Table(struct mbr_partition *from ,struct mbr_partition *to)
{
    memcpy(to, from, sizeof(struct mbr_partition));
    memset(from, 0x00, sizeof(struct mbr_partition));
}

static void tidy_up_MBR_Table(struct partition *partition)
{
    uint8_t state[4];
    for(int i = 0; i < 4; i++)
        state[i] = 1 - check_empty_MBR_Table(partition,i);

    for(int j = 0; j < 3; j++){
        for(int k = 0; k < 3; k++){
            if(state[k] == 0 && state[k+1] == 1){
                move_MBR_Table(&partition->mbr_partition[k+1],&partition->mbr_partition[k]);
                state[k]   = 1;
                state[k+1] = 0;
            }
        }
    }
}

static int mbr_partition_table_load(struct block_device *device,struct partition *partition) 
{
    if (device == NULL || partition == NULL) return -1;
    char *data = kmalloc(512,GFP_KERNEL);
    if(data == NULL){
        printk(KERN_INFO "no memory,need size %d\n",512);
         return -1;
    }
    if(trans_data_to_blk_dev(device,0,512,data,REQ_OP_READ) == NULL){
        printk(KERN_INFO "can not get data from disk\n");
        kfree(data);
        return -1;
    }
    memcpy(&partition->mbr_partition[0] , data + 446, sizeof(struct mbr_partition)*4); // 复制前4个分区表到mbr_partition数组
    for(int i =0;i<4;i++){
        MBR_partition_table_fix_endian(&partition->mbr_partition[i]);
    }
    kfree(data);
    return 0;
}

static int mbr_partition_table_update(struct block_device *device,struct partition *partition) 
{
    if(device == NULL || partition == NULL) return -1;
    char *data = kmalloc(512,GFP_KERNEL);
    if(data == NULL) return -1;
    for(int i =0;i<4;i++){
        MBR_partition_table_fix_endian(&partition->mbr_partition[i]);
    }
    memcpy(data + 446, &partition->mbr_partition[0], sizeof(struct mbr_partition)*4);
    data[0x01FE] = 0x55;
    data[0x01FF] = 0xAA;                                                                //添加标识位    
    if(trans_data_to_blk_dev(device,0,512,data,REQ_OP_WRITE) == NULL){
        kfree(data);
         return 0;
    }
    else{
        kfree(data);
        return -1;        
    }
}

static int mbr_table_delete(struct block_device *device,struct partition *partition,int number,enum block_device_flags_t flags)
{
    if(device == NULL || partition == NULL) {
        printk("This device has not init\n\r");
        return -1;
    }
    if(flags != BLOCK_DEVICE_FLAG_MBR && flags!= BLOCK_DEVICE_FLAG_PROTECTIVE_MBR){
        printk("It's not a mbr device\n\r");
        return -1;
    }
    if(number < 0 || number > 3){
        printk("partition number is out of range\n\r");
        return -1;
    }
    if(mbr_partition_table_load(device,partition) < 0) return -1;           //读取mbr分区表
    struct mbr_partition *mbr_partition = &partition->mbr_partition[number];
    if(check_empty_MBR_Table(partition,number) == 1){
        printk("This is a empty partition table slot\n\r");
        return -1;
    }
    else
        memset(mbr_partition, 0x00, sizeof(struct mbr_partition));
    tidy_up_MBR_Table(partition);
    mbr_partition_table_update(device,partition);
    return 0;
}



static int mbr_partition_table_format(struct block_device *device,struct partition *partition,uint32_t size,uint8_t type) 
{
    if(device == NULL || partition == NULL) {
        printk(KERN_INFO "This device has not init\n");
        return -1;
    }
    if(mbr_partition_table_load(device,partition) < 0) 
    {
        printk(KERN_INFO "can not load mbr partition\n");
        return -1;           //读取mbr分区表
    }
    size = size /8 *8;  
    if(size < 0) return -1;
    
    int partition_number = 0;
    struct mbr_partition *mbr_partition = get_empty_MBR_Table(partition,&partition_number);   //获取空闲的MBR分区表槽位
    if(mbr_partition == NULL){
        printk(KERN_INFO "this storage device can not builded more partition table any more\n");
        return -1;
    }
    //计算分区起始地址
    uint32_t start_address;
    if(partition_number - 1 >= 0){     
        start_address = get_mbr_partition_start_address(partition,partition_number - 1);
        printk(KERN_INFO "GET last table start address = %d last_size\n",start_address);
    }
    else{
        printk(KERN_INFO "no last table\n");
        start_address = INVALID_PARTITION;
    }
    if (start_address == INVALID_PARTITION){
        printk(KERN_INFO "it's the first partition\n");
        if(device->bd_nr_sectors  < 2048){
           printk(KERN_INFO "this disk size is too small");
            return -1;
        }
        else {
            start_address = 2048  ;                                                             //表示前方没有分区表，则从2048号扇区开始(需要预留一定空间)
            printk(KERN_INFO "dected stand storage device,use 2048 as start address\n");
        }
    }
    else {
        uint32_t last_size = get_mbr_partition_size(partition,partition_number - 1);               //获取前一个分区的结束地址，并加上该分区大小
        start_address += last_size;               
        printk(KERN_INFO "GET last table size = %d block\n",last_size);
    }

    if(start_address%8 > 0) //4k对其并向上对其
    start_address = start_address - (start_address%8) + 8;


    //计算分区结束地址
    uint32_t end_address;
    if(partition_number + 1 < 4)
        end_address = get_mbr_partition_start_address(device,partition_number + 1);        //获取后一个分区的起始地址作为当前分区的结束地址
    else 
        end_address = INVALID_PARTITION;
    if(end_address == INVALID_PARTITION){
        end_address =device->bd_nr_sectors - 33 -1;                                          //表示如果后方没有分区表，则直接使用扇区数量减去33扇区(gpt分区表预留)作为最后一个分区的起始地址
    }
    if(end_address < start_address){
        printk(KERN_INFO "not support this storage device,storage siz is too small\n");
        return -1;
    }
    if (end_address <= start_address + size) {                                                 
        printk(KERN_INFO "Partition size %d is too large, adjusting to fit available space.\n", size);
        size = end_address - start_address;
        printk(KERN_INFO "New partition size: %d\n", size);
    }
    if(size <= 0 ){
        printk(KERN_INFO "this size: %d of block  is not work\n",size);
        return -1;
    }
    MBR_partition_table_create(mbr_partition, 0x00, start_address, size, type);              //创建新的MBR分区,不可引导
    tidy_up_MBR_Table(partition);                                                               //整理MBR分区表保证分区表相互贴合
    mbr_partition_table_update(device,partition);                                                      //更新MBR分区表到磁盘
    return 0;
}

static void  show_mbr_partition_table_info(struct mbr_partition *mbr_partition)
{
    if(mbr_partition == NULL) return;

    if(mbr_partition->boot_ind == 0x80)
        printk(KERN_INFO"bootable\n\r");
    else
        printk(KERN_INFO"not bootable\n\r");
    printk(KERN_INFO"start_address = %d\n\r",mbr_partition->start_lba);
    printk(KERN_INFO"size          = %d(%dKB)(%dMB)\n\r",mbr_partition->nr_sectors,mbr_partition->nr_sectors/2,mbr_partition->nr_sectors/2048);
    
    if(mbr_partition->sys_ind == 0x0B)
    printk(KERN_INFO"type          = FAT\n\r");
    else if(mbr_partition->sys_ind == 0x83)
    printk(KERN_INFO"type          = EXT\n\r");
    else if(mbr_partition->sys_ind == 0x07)
    printk(KERN_INFO"type          = NTFS\n\r");
    else
    printk(KERN_INFO"type          = unknown\n\r");
}














struct partition* get_partition_from_device(struct block_device *bdev){
    if(bdev == NULL) return NULL;
    struct partition *device_partition = kmalloc(sizeof(struct partition),GFP_KERNEL);
    if(device_partition == NULL) return -ENOMEM;
    if(mbr_partition_table_load(bdev,device_partition) < 0){
        kfree(device_partition);
        return -EIO;
    }
    return device_partition;
}

#define mbr_partiton_startaddress 0x01
#define mbr_partiton_size         0x02
#define mbr_partiton_magic        0x03

uint32_t get_partiton_data(struct partition* partition,int number,uint32_t flag)
{
    if(partition == NULL) return 0;
    if(flag == mbr_partiton_startaddress)
        return get_mbr_partition_start_address(partition, number);
    if(flag == mbr_partiton_size)
        return get_mbr_partition_size(partition,number);
    if(flag == mbr_partiton_magic)
        return get_mbr_partition_type(partition,number);
}

#define  FAT32 0x0B
#define  EXT   0x83
#define  NTFS  0x07

int create_mbr_artiton(struct block_device *bdev,size_t size,uint8_t type)
{
    struct partition device_partition;
    return mbr_partition_table_format(bdev,&device_partition,size,type); 
}

int get_partition_type(struct block_device *bdev)
{
    enum block_device_flags_t flags;
    char *buffer = kmalloc(512,GFP_KERNEL);
    if(buffer == NULL) return -ENOMEM;
    detect_partition_table(bdev,buffer,&flags);
    kfree(buffer);
    return flags;
}

static int disk_show_mbr_info(struct block_device *bdev)
{
    struct partition *partition = get_partition_from_device(bdev);
    if(partition == NULL) return -1;
     
    for(int i = 0; i < 4; i++){
        if(check_empty_MBR_Table(partition,i) == 0)
        {
            printk(KERN_INFO"-------------------------------------------\n\r");
            printk(KERN_INFO"partition %d:\n\r",i);
            show_mbr_partition_table_info(&partition->mbr_partition[i]);
        }
    }
    kfree(partition);
    return 0;
}


