#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include <linux/ext2_fs.h>
#include <linux/mempool.h>
#include <linux/swab.h>
#include "ext2.h"


static void my_end_io(struct bio *bio){
    bio_put(bio);
}
static char * trans_data_to_blk_dev(struct block_device *bdev, sector_t block, unsigned size, char *data, int mod){
    struct page *page = virt_to_page(data);  
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



static struct ext2_super_block  * ext2_superblock_create(struct block_device *bdev,char *name,int sector_size)
{
    if(bdev == NULL) return NULL;
    struct ext2_super_block * super_block = kmalloc(sizeof(struct ext2_super_block),GFP_KERNEL); //分配superblock
    if(!super_block) return NULL;

    sector_t num_sectors   =  bdev_nr_sectors(bdev);             //获取扇区数量
    int      block_size_shift = ilog2(sector_size) - 10;         // 512字节扇区 → -1
    uint32_t block_size = 1024 << block_size_shift;              // 实际块大小（字节）
    uint32_t blocks_count  =  num_sectors  / 2 ;                 // 一个块为1024字节，使用逻辑块数量/2
    uint32_t inodes_count  =  blocks_count / 4;
    uint32_t blocks_per_group = block_size * 8;                  // 每组块数（通常是块大小*8）
    uint32_t groups_count = (blocks_count + blocks_per_group - 1) / blocks_per_group;
    uint32_t inodes_per_group = (inodes_count + groups_count - 1) / groups_count;

    memset(super_block, 0, sizeof(struct ext2_super_block));

    super_block->s_inodes_count = inodes_count;
    super_block->s_blocks_count = blocks_count;
    super_block->s_r_blocks_count = super_block->s_blocks_count/20;             //为超级用户保留的块数为总块数的5%
    super_block->s_free_blocks_count = (super_block->s_blocks_count - 1 ) / 8 * 8;  
    super_block->s_free_inodes_count = super_block->s_inodes_count - 1;
    super_block->s_first_data_block  = (block_size == 1024) ? 1 : 0   ;
    super_block->s_log_block_size    = ilog2(block_size) - 10;
    super_block->s_log_frag_size     = block_size;
    super_block->s_blocks_per_group  = blocks_per_group;
    super_block->s_frags_per_group   = blocks_per_group;
    super_block->s_inodes_per_group  = inodes_per_group;
    super_block->s_mtime         =   ktime_get_real_seconds();
    super_block->s_wtime         =   super_block->s_mtime;
    super_block->s_mnt_count     =   0;
    super_block->s_max_mnt_count =   20;                      // 常见默认值
    super_block->s_magic         =   0xEF53;                  // EXT2魔数
    super_block->s_state         =   1;                       // EXT2_VALID_FS
    super_block->s_errors        =   1;                       // 默认继续挂载
    super_block->s_minor_rev_level = 0;
    super_block->s_lastcheck       = super_block->s_mtime;
    super_block->s_checkinterval   = 86400;                  // 默认每天检查一次
    super_block->s_creator_os  =  0;                           // 版本和OS// 0表示Linux
    super_block->s_rev_level   =  1;                           // 动态版本
    super_block->s_def_resuid  =  0;                           // 保留字段（可选）
    super_block->s_def_resgid  =  0;
    super_block->s_first_ino       =  11;                      // Inode相关 第一个非保留inode
    super_block->s_inode_size      = 128;                     // 标准inode大小
    super_block->s_block_group_nr  =   0;                       //主超级块为0
    super_block->s_feature_compat    = 0;
    super_block->s_feature_incompat  = 0;
    super_block->s_feature_ro_compat = 0;
    generate_random_uuid(super_block->s_uuid);               // 随机生成UUID
    strncpy(super_block->s_volume_name, name, 15);super_block->s_volume_name[15] = '\0'; // 确保终止
    super_block->s_algorithm_usage_bitmap = 0; 
    super_block->s_prealloc_blocks        = 0;               // 禁用普通文件预分配
    super_block->s_prealloc_dir_blocks    = 1;               // 目录预分配1个块（常见默认值）
    super_block->s_padding1               = 0;               // 填充字段置零
    memset(super_block->s_reserved,super_block->s_padding1,sizeof(super_block->s_reserved));
    return super_block;
}

static struct ext2_super_block * read_ext2_superblock(struct block_device *bdev,sector_t offset){
    struct ext2_super_block * super_block = kmalloc(sizeof(struct ext2_super_block),GFP_KERNEL); //分配superblock
    if    (!super_block)                       return NULL;
    memset( super_block , 0 , sizeof( struct ext2_super_block ));												
    int err = trans_data_to_blk_dev(bdev, offset, sizeof(struct ext2_super_block), super_block, REQ_OP_READ);
    if( err < 0 ){
        vfree (super_block);
        return NULL;
    }
    return super_block;
}

static int writ_ext2_superblock(struct block_device *bdev,struct ext2_super_block *super_block,sector_t offset){    
    if(super_block  == NULL) return -1;
	int err = trans_data_to_blk_dev(bdev, offset, sizeof(struct ext2_super_block), super_block, REQ_OP_WRITE);
    if(err < 0)
        return -1;
    return 0;
}

static struct ext2_super_block * read_main_ext2_superblock(struct block_device *bdev){
    if(bdev == NULL) return NULL;
    struct ext2_super_block * super_block = read_ext2_superblock(bdev,2);
    if(super_block == NULL) return NULL;
    if(super_block->s_magic == 0xEF53)
        return super_block;
    printk(KERN_INFO "It is not ext2 superblock\n");
    return NULL;
}

static int write_main_ext2_superblock(struct block_device *bdev,struct ext2_super_block *super_block){
    if(bdev == NULL) return NULL;
    return writ_ext2_superblock(bdev,super_block,2);
}
static int __ext2_does_free_inode_have(struct ext2_super_block *super_block){
    if(super_block == NULL) return 0;
    return super_block->s_free_inodes_count >= super_block->s_r_blocks_count;
}











static __u32 __ext2_get_file_type(struct ext2_inode *node){
    if(!node) return 0;
    return node->i_mode&S_IFMT;
} 
static __u64 __ext2_get_file_size(const struct ext2_inode *inode) {
    if (!inode) return 0;  
    if(ext2_get_file_type(inode) == S_IFDIR) //如果是目录
        return inode->i_size;
    else
        return inode->i_dir_acl << 32 | (inode->i_size);
}
static void __ext2_set_file_size(struct ext2_inode *inode,__u64 size){
    inode->i_size = size;
    if(ext2_get_file_type(inode) != S_IFDIR){
        inode->i_dir_acl = size >> 32;
    }
}










static sector_t __ext2_get_group_size(struct ext2_super_block *sb){
    return  sb->s_blocks_per_group * ((sb->s_blocks_per_group / 8 ) /512);
}
static sector_t __ext2_get_group_start_address(struct ext2_super_block *sb , uint32_t group_number){ //获取ext2组所在起始扇区
    return 2 + __ext2_get_group_size(sb) *group_number;
}













static int __read_block512(struct block_device *bdev, sector_t sector, void *buf) {
    return trans_data_to_blk_dev(bdev, sector, 512, buf, REQ_OP_READ);
}
static int __read_write512(struct block_device *bdev, sector_t sector, void *buf) {
    return trans_data_to_blk_dev(bdev, sector, 512, buf, REQ_OP_WRITE);
}
static struct ext2_inode* __ext2_new_inode_create(struct ext2_super_block *sb,__u16 uid,u16 gid){
    struct ext2_inode *inode = kmalloc(sb->s_inode_size, GFP_KERNEL);
    if (!inode) return NULL;
    memset(inode,0,sizeof(struct ext2_inode));
    inode->i_uid = uid;
    inode->i_atime = jiffies;
    inode->i_ctime = inode->i_atime;
    inode->i_mtime = inode->i_atime;
    inode->i_gid =gid;
    return inode;
}
static struct ext2_inode*__ext2_read_inode(struct block_device *bdev, struct ext2_super_block *sb, unsigned int ninode)
{
    struct ext2_inode *inode = kmalloc(sb->s_inode_size, GFP_KERNEL);
    if (!inode) return NULL;


    return inode;
}












size_t get_global_heap_size(void);

static void partiton_test()
{
    struct block_device *bdev = blkdev_get_by_path("/dev/sda0", FMODE_READ | FMODE_WRITE, NULL);
    if(bdev == NULL){
        printk(KERN_INFO "can not open /dev/sda0\n");
        return -1;
    }
    printk(KERN_INFO "open /dev/sda0 sucessful\n");

    struct ext2_super_block  * block = ext2_superblock_create(bdev,"storage001",512);
    struct ext2_super_block  * block1;
    if(block == NULL) return -1;
    int flag = write_main_ext2_superblock(bdev,block);
    if(flag >= 0)
    {
        block1 = read_main_ext2_superblock(bdev);
        if(block1 != NULL)
        print_memory(block1,sizeof(struct ext2_super_block));  
    }
}


// late_initcall(partiton_test);


