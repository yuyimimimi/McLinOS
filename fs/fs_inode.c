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


struct inode* new_inode(struct super_block *sb){
    struct inode *inode = (struct inode *)kmalloc(sizeof(struct inode), GFP_KERNEL);
    if (inode == NULL) return NULL;
	memset(inode,0,sizeof(struct inode));
    inode->i_mode    = S_IFCHR | 0777;
    time64_t now        = jiffies;
    inode->i_sb       = sb;
	atomic_set(&inode->i_count,1);
    return inode;
}
EXPORT_SYMBOL(new_inode);

void destroy_inode(struct inode *node){
	if(node != NULL)
	kfree(node);
}
EXPORT_SYMBOL(destroy_inode);

void inode_get(struct inode *inode){
    atomic_inc(&inode->i_count);  
}
EXPORT_SYMBOL(inode_get);

/**
 * inode_put - Decrease inode reference count and free if unused
 * @inode: pointer to the inode to release
 *
 * This function decreases the reference count of the given inode.
 * If the count reaches zero, it destroys the inode structure,
 * freeing the associated memory.
 *
 * Typically used when a dentry or file is released and the inode
 * is no longer referenced elsewhere.
 *
 * 中文说明：
 * inode_put - 减少 inode 引用计数，并在无引用时释放
 * @inode: 要释放的 inode 指针
 *
 * 此函数用于减少 inode 的引用计数。当计数减少到 0 时，
 * 会调用 destroy_inode() 销毁该 inode 并释放其占用的内存。
 *
 * 常用于 dentry 或文件对象释放之后，当 inode 不再被使用时。
 * 
 * 它只在inode未添加到vfs中时使用
 */
void inode_put(struct inode *inode){
    if(inode!= NULL)
    if (atomic_dec_and_test(&inode->i_count) || inode->i_count.counter < 0) {  
        pr_info("remove inode\n");
        destroy_inode(inode);  
    }
}
EXPORT_SYMBOL(inode_put);

