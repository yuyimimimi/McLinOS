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

void inode_put(struct inode *inode){
    if (atomic_dec_and_test(&inode->i_count)) {  
        destroy_inode(inode);  
    }
}
EXPORT_SYMBOL(inode_put);

