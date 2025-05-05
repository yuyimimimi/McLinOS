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











// struct dentry *d_alloc_anon(struct super_block *sb)
// {
// 	return __d_alloc(sb, NULL);
// }
// EXPORT_SYMBOL(d_alloc_anon);



// void __d_drop(struct dentry *dentry)
// {
// 	if(dentry->d_iname == dentry->d_name.name){
// 		kfree(dentry);
// 	}
// 	else{
// 		kfree(dentry->d_name.name);
// 		kfree(dentry);
// 	}
// }
// EXPORT_SYMBOL(__d_drop);













// void d_delete(struct dentry * dentry){
// 	spin_lock(&dentry->d_parent->d_lock);	
// 	if (dentry->d_parent) {
//         hlist_del(&dentry->d_sib);
//     }
// 	spin_unlock(&dentry->d_parent->d_lock);	

// 	inode_put(&dentry->d_inode);
// 	if(dentry->d_op->d_release != NULL){
// 		dentry->d_op->d_release(dentry);
// 	}
// 	if(dentry != NULL)
// 	__d_drop(dentry);

// }
// EXPORT_SYMBOL(d_delete)
// int d_add(struct dentry *dentry, struct inode *inode){
// 	dentry->d_inode = inode;



	
// }
// EXPORT_SYMBOL(d_add);
// int simple_unlink(struct inode *dir ,struct dentry * dentry){
// 	return 0;
// }
// EXPORT_SYMBOL(simple_unlink);

// void d_put(struct dentry *dentry){
// 	if(dentry)
// 	{
// 		if(dentry->d_iname != dentry->d_name.name){
// 			kfree(dentry->d_name.name);
// 		}
// 		if(dentry->d_inode){
// 			inode_put(dentry->d_inode);
// 		}
// 		kfree(dentry);		
// 	}
// }
// EXPORT_SYMBOL(struct dentry);





// //superblock

// struct super_block *alloc_super(struct block_device *bdev){
// 	struct super_block * sb = kmalloc(sizeof(struct super_block),GFP_KERNEL);
//     if(sb == NULL) return NULL;
// 	memset(sb,0,sizeof(struct super_block));
// 	INIT_LIST_HEAD(&sb->s_list);

// 	sb->s_bdev = bdev;
// 	mutex_init(&sb->s_vfs_rename_mutex);
// 	INIT_HLIST_HEAD(&sb->s_pins);

// }
// EXPORT_SYMBOL(alloc_super);
// void put_super(struct super_block *sb){
// 	if(sb !=  NULL)
// 	kfree(sb);	
// }
// EXPORT_SYMBOL(put_super);








