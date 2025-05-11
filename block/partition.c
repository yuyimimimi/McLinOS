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

/**
 * detect_partition_table - Detect the partition table of the block device
 * @device: pointer to the block device to check
 * @block_buffer: buffer to hold the data read from the block device
 * @flags: pointer to a variable to store the partition table type flags
 *
 * This function attempts to detect the partition table type of a block device
 * by reading the first two Logical Block Addresses (LBA) and checking for 
 * signatures that indicate the partitioning scheme.
 *
 * Steps:
 * 1. Initially, the device is marked as uninitialized (`BLOCK_DEVICE_FLAG_NOT_INITIALIZED`).
 * 2. The first 512 bytes of LBA 0 are read to check for the MBR (Master Boot Record) signature.
 *    If the signature (`0x55AA` at the end of the sector) is found, the device is flagged as MBR.
 * 3. The first 512 bytes of LBA 1 are read to check for the GPT (GUID Partition Table) header.
 *    If the GPT header is found, the partition table type is set to GPT, or if an MBR was detected,
 *    it is marked as a protective MBR.
 *
 * 中文说明：
 * detect_partition_table - 检测块设备的分区表类型
 * @device: 指向要检测的块设备的指针
 * @block_buffer: 用于存储从块设备读取的数据的缓冲区
 * @flags: 指向用于存储分区表类型标志的变量的指针
 *
 * 本函数通过读取块设备的前两个逻辑块地址（LBA）并检查是否存在分区表签名，
 * 来尝试检测块设备的分区表类型。
 *
 * 步骤：
 * 1. 初始时，将设备标记为未初始化（`BLOCK_DEVICE_FLAG_NOT_INITIALIZED`）。
 * 2. 读取 LBA 0 的前 512 字节，检查是否存在 MBR（主引导记录）签名。
 *    如果发现签名（LBA 末尾的 `0x55AA`），则将设备标记为 MBR。
 * 3. 读取 LBA 1 的前 512 字节，检查是否存在 GPT（GUID 分区表）头。
 *    如果发现 GPT 头，则将分区表类型设置为 GPT，若之前检测到 MBR，则标记为保护性 MBR。
 */
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
EXPORT_SYMBOL(detect_partition_table);





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

/**
 * mbr_table_delete - 删除 MBR 分区表中的指定分区
 * @device: 指向块设备的指针
 * @partition: 指向分区结构的指针
 * @number: 要删除的分区号（0-3）
 * @flags: 当前设备的分区表标志，需为 MBR 或 PROTECTIVE_MBR
 *
 * 该函数删除指定的 MBR 分区表中的分区。如果该分区号有效且设备支持 MBR 格式，
 * 将清空对应的分区数据，并更新分区表。
 *
 * 返回值：
 * - 0 表示成功，-1 表示失败。
 *
 * 中文说明：
 * mbr_table_delete - 删除 MBR 分区表中的指定分区
 * @device: 指向块设备的指针
 * @partition: 指向分区结构的指针
 * @number: 要删除的分区号（0 到 3）
 * @flags: 当前设备的分区表标志，必须为 MBR 或 PROTECTIVE_MBR
 *
 * 该函数删除指定 MBR 分区表中的分区。如果分区号有效且设备支持 MBR 格式，
 * 将清空对应的分区，并更新分区表。
 *
 * 返回值：
 * - 成功时返回 0，失败时返回 -1。
 */
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
EXPORT_SYMBOL(mbr_table_delete);


/**
 * mbr_partition_table_format - Format the partition table on the block device using MBR scheme
 * @device: pointer to the block device
 * @partition: pointer to the partition data structure
 * @size: the size of the partition to create (in sectors)
 * @type: the type of the partition (e.g., primary, logical, etc.)
 *
 * This function attempts to format a partition table on a block device using the MBR (Master Boot Record)
 * partitioning scheme. It first loads the existing MBR partition table, determines the available space for 
 * the new partition, and then creates a new partition entry in the MBR partition table. The partition's start 
 * and end addresses are calculated, and the partition is aligned to 4K boundaries.
 * 
 * The partition size is adjusted to fit within the available space on the disk. If the size provided is too 
 * large, it will be reduced to fit. The partition table is then updated and written back to the device.
 *
 * 中文说明：
 * mbr_partition_table_format - 使用 MBR 方案格式化块设备的分区表
 * @device: 指向块设备的指针
 * @partition: 指向分区数据结构的指针
 * @size: 要创建的分区大小（以扇区为单位）
 * @type: 分区类型（如主分区、逻辑分区等）
 *
 * 本函数尝试使用 MBR（主引导记录）分区方案格式化块设备上的分区表。首先，它加载现有的 MBR 
 * 分区表，确定新分区可用的空间，然后在 MBR 分区表中创建一个新的分区条目。分区的起始地址和 
 * 结束地址将根据磁盘的可用空间来计算，并且分区会对齐到 4K 边界。
 * 
 * 如果提供的分区大小过大，函数会自动调整为适合磁盘的大小。最后，分区表会更新并写回到设备。
 */
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
EXPORT_SYMBOL(mbr_partition_table_format);


/**
 * show_mbr_partition_table_info - Display information about a given MBR partition
 * @mbr_partition: pointer to the MBR partition structure
 *
 * This function prints information about an MBR (Master Boot Record) partition. It displays whether the partition 
 * is bootable, its starting address, size in sectors, size in kilobytes and megabytes, as well as the type of the 
 * partition based on its system identifier.
 * 
 * If the partition is bootable (boot_ind == 0x80), it will print "bootable". If it is not, it will print "not bootable".
 * The partition's type is determined by the system identifier (sys_ind), with common values being:
 * - 0x0B: FAT type partition
 * - 0x83: EXT type partition
 * - 0x07: NTFS type partition
 * If the partition's system identifier does not match these values, it will be labeled as "unknown".
 *
 * 中文说明：
 * show_mbr_partition_table_info - 显示 MBR 分区的相关信息
 * @mbr_partition: 指向 MBR 分区结构的指针
 *
 * 本函数打印给定 MBR（主引导记录）分区的信息。包括分区是否可引导，起始地址，大小（以扇区为单位），
 * 大小（以千字节和兆字节为单位），以及根据系统标识符确定的分区类型。
 * 
 * 如果分区是可引导的（boot_ind == 0x80），则打印“bootable”。如果不可引导，则打印“not bootable”。
 * 分区类型由系统标识符（sys_ind）决定，常见的值如下：
 * - 0x0B：FAT 类型分区
 * - 0x83：EXT 类型分区
 * - 0x07：NTFS 类型分区
 * 如果分区的系统标识符与这些值不匹配，则标记为“unknown”。
 */
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



/**
 * get_partition_from_device - Retrieve the partition information from a block device
 * @bdev: pointer to the block device structure
 *
 * This function attempts to load the partition table of the specified block device 
 * and returns a pointer to the partition structure. If the block device is invalid 
 * or if the partition table cannot be loaded, the function will return an error code.
 *
 * Steps:
 * 1. Allocates memory for a partition structure.
 * 2. Loads the MBR (Master Boot Record) partition table for the specified block device.
 * 3. Returns a pointer to the partition structure if successful, otherwise returns an error code.
 *
 * 中文说明：
 * get_partition_from_device - 从块设备获取分区信息
 * @bdev: 指向块设备结构的指针
 *
 * 本函数尝试加载指定块设备的分区表，并返回指向分区结构的指针。如果块设备无效或无法加载分区表，
 * 则函数会返回错误代码。
 *
 * 步骤：
 * 1. 为分区结构分配内存。
 * 2. 加载指定块设备的 MBR（主引导记录）分区表。
 * 3. 如果成功，返回分区结构的指针；如果失败，返回错误代码。
 */
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
EXPORT_SYMBOL(get_partition_from_device);

#define mbr_partiton_startaddress 0x01
#define mbr_partiton_size         0x02
#define mbr_partiton_magic        0x03


/**
 * get_partiton_data - Retrieve specific partition data based on the flag
 * @partition: pointer to the partition structure
 * @number: partition number to get data for
 * @flag: flag to specify what partition data to retrieve
 *
 * This function retrieves specific partition data based on the given flag.
 * Depending on the flag value, it returns different pieces of information 
 * related to the specified partition. 
 * The flags used in this function correspond to partition start address, 
 * size, and type as per the MBR (Master Boot Record) partition table.
 *
 * Steps:
 * 1. Checks if the partition pointer is valid.
 * 2. Based on the flag, retrieves the corresponding partition data.
 *    - `mbr_partiton_startaddress`: Retrieves the partition's start address.
 *    - `mbr_partiton_size`: Retrieves the partition's size.
 *    - `mbr_partiton_magic`: Retrieves the partition's type.
 * 3. Returns the corresponding partition data or 0 if the partition is NULL or flag is unrecognized.
 *
 * 中文说明：
 * get_partiton_data - 获取特定分区数据，依据给定的标志
 * @partition: 指向分区结构的指针
 * @number: 要获取数据的分区号
 * @flag: 指定获取哪个分区数据的标志
 *
 * 本函数根据给定的标志检索特定的分区数据。
 * 根据标志的值，返回与指定分区相关的不同信息。
 * 这些标志对应 MBR（主引导记录）分区表中的分区起始地址、大小和类型。
 *
 * 步骤：
 * 1. 检查分区指针是否有效。
 * 2. 根据标志，检索相应的分区数据：
 *    - `mbr_partiton_startaddress`：获取分区的起始地址。
 *    - `mbr_partiton_size`：获取分区的大小。
 *    - `mbr_partiton_magic`：获取分区的类型。
 * 3. 返回对应的分区数据，或者在分区为 NULL 或标志无法识别的情况下返回 0。
 */
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
EXPORT_SYMBOL(get_partiton_data);


#define  FAT32 0x0B
#define  EXT   0x83
#define  NTFS  0x07

/**
 * create_mbr_artiton - 创建 MBR 分区
 * @bdev: 指向块设备的指针
 * @size: 分区大小（单位为字节）
 * @type: 分区类型
 *
 * 该函数调用 mbr_partition_table_format 函数来格式化并创建 MBR 分区。
 * 它首先初始化分区数据结构，并传递到 mbr_partition_table_format 函数进行实际的分区创建。
 * 
 * 中文说明：
 * create_mbr_artiton - 创建 MBR 分区
 * @bdev: 指向块设备的指针
 * @size: 分区大小（字节）
 * @type: 分区类型
 *
 * 该函数调用 mbr_partition_table_format 函数格式化并创建 MBR 分区。
 * 它会初始化一个分区结构并将其传递给 mbr_partition_table_format 进行实际的分区创建。
 */
int create_mbr_artiton(struct block_device *bdev,size_t size,uint8_t type)
{
    struct partition device_partition;
    return mbr_partition_table_format(bdev,&device_partition,size,type); 
}
EXPORT_SYMBOL(create_mbr_artiton);


/**
 * get_partition_type - 获取块设备的分区类型
 * @bdev: 指向块设备的指针
 *
 * 该函数检测给定的块设备的分区表类型，支持 MBR 和 GPT 格式的检测。
 * 它使用 detect_partition_table 函数读取设备的分区表并返回分区类型。
 *
 * 返回值：
 * - 返回块设备的分区表标志，值为 BLOCK_DEVICE_FLAG_MBR, BLOCK_DEVICE_FLAG_GPT 等。
 *
 * 中文说明：
 * get_partition_type - 获取块设备的分区类型
 * @bdev: 指向块设备的指针
 *
 * 本函数检测给定块设备的分区表类型，支持 MBR 和 GPT 格式的分区表。
 * 它通过调用 detect_partition_table 函数来读取设备的分区表并返回对应的分区类型。
 *
 * 返回值：
 * - 返回块设备的分区标志，如 BLOCK_DEVICE_FLAG_MBR 或 BLOCK_DEVICE_FLAG_GPT。
 */
int get_partition_type(struct block_device *bdev)
{
    enum block_device_flags_t flags;
    char *buffer = kmalloc(512,GFP_KERNEL);
    if(buffer == NULL) return -ENOMEM;
    detect_partition_table(bdev,buffer,&flags);
    kfree(buffer);
    return flags;
}

/**
 * disk_show_mbr_info - 显示磁盘的 MBR 分区信息
 * @bdev: 指向块设备的指针
 *
 * 该函数从指定的块设备中获取分区数据，并显示每个分区的 MBR 信息。
 * 它遍历最多 4 个分区，使用 check_empty_MBR_Table 检查每个分区是否为空，
 * 如果不为空，调用 show_mbr_partition_table_info 显示分区信息。
 *
 * 返回值：
 * - 返回 0 表示成功，-1 表示失败。
 *
 * 中文说明：
 * disk_show_mbr_info - 显示磁盘的 MBR 分区信息
 * @bdev: 指向块设备的指针
 *
 * 本函数从指定的块设备获取分区数据并显示每个分区的 MBR 信息。
 * 它遍历最多 4 个分区，使用 check_empty_MBR_Table 判断分区是否为空，
 * 如果分区不为空，则调用 show_mbr_partition_table_info 显示该分区的信息。
 *
 * 返回值：
 * - 成功返回 0，失败返回 -1。
 */
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


