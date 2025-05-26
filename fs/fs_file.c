#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>

struct file * f_get(struct dentry * dentry)
{
    struct inode *inode = dentry->d_inode;

    spin_lock(&inode->i_lock);
    struct file*file_node = kmalloc(sizeof(struct file),GFP_KERNEL);
    if(file_node == NULL)
    {
        spin_unlock(&inode->i_lock);
        return ERR_PTR(-ENOMEM);
    }
    inode_get(inode);
    
    spin_unlock(&inode->i_lock);
    memset(file_node,0,sizeof(struct file));
    mutex_init(&file_node->f_ref);
    mutex_init(&file_node->f_lock);    
    spin_lock_init(&file_node->f_slock);    
    file_node->f_op   = inode->i_fop;
    file_node->f_inode = inode;
    file_node->f_path  = dentry->d_name.name; 
    return file_node;
}
EXPORT_SYMBOL(f_get);

struct file* f_put(struct file * file){
    inode_put(file->f_inode); //原子性操作，不需要加锁
    kfree(file);
}
EXPORT_SYMBOL(f_put);
