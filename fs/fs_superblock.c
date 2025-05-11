#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/dcache.h>
#include <linux/export.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <linux/atomic.h>




struct super_block *alloc_super(struct block_device *bdev)
{
	struct super_block * sb = kmalloc(sizeof(struct super_block),GFP_KERNEL);
    if(sb == NULL) 
    return -ENOMEM;
	memset(sb,0,sizeof(struct super_block));
	INIT_LIST_HEAD(&sb->s_list);
	sb->s_bdev = bdev;
    INIT_HLIST_NODE(&sb->s_instances);
    INIT_LIST_HEAD(&sb->s_mounts);
	mutex_init(&sb->s_vfs_rename_mutex);
	sb->s_blocksize = 512;
	INIT_HLIST_HEAD(&sb->s_pins);
}
EXPORT_SYMBOL(alloc_super);

void put_super(struct super_block *sb){
	if(sb !=  NULL)
	kfree(sb);	
}
EXPORT_SYMBOL(put_super);


