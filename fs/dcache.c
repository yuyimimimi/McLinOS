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
#include <linux/init.h>



/**
 * dentry_rename - Rename a dentry to a new name
 * @d: pointer to the dentry to rename
 * @name: new name to assign to the dentry
 *
 * This function renames the given dentry by assigning it a new name.
 * It first checks if the dentry or the new name is NULL and returns an error if so.
 * It then attempts to allocate memory for the new name and copies the name into it.
 *
 * If the dentry's current name is inlined (i.e., `d_name.name == d_iname`), 
 * it tries to copy the new name into the inline buffer, provided it fits within 
 * the maximum allowed length for inline names.
 * 
 * If the name does not fit into the inline buffer or if it is not inline, 
 * it allocates memory for the new name and replaces the old one.
 *
 * 中文说明：
 * dentry_rename - 重命名 dentry
 * @d: 要重命名的 dentry 指针
 * @name: 要赋予 dentry 的新名字
 *
 * 本函数通过分配新的内存空间来重命名给定的 dentry。
 * 首先检查 dentry 或新名字是否为 NULL，若是则返回错误。
 * 然后尝试分配内存并将新名字拷贝到其中。
 *
 * 如果 dentry 当前的名字是内联的（即 `d_name.name == d_iname`），
 * 它会尝试将新名字拷贝到内联缓冲区中，前提是新名字的长度不会超过内联名字的最大长度。
 *
 * 如果名字不能适应内联缓冲区，或者名字不是内联的，
 * 它会为新名字分配内存并替换掉旧名字。
 * 
 */
int dentry_rename(struct dentry *d,char *name)
{
	if(d == NULL || name == NULL) 
		return -1;
    
	int new_name_length = strlen(name);
	char *new_name = kmalloc(new_name_length ,GFP_KERNEL); //先尝试申请内存，如果后续失败不会破坏现场
    if(new_name == NULL) return -ENOMEM;
    strcpy(new_name,name);

    if(d->d_name.name == d->d_iname || d->d_name.name == NULL) 
	{
        if( new_name_length + 1 < DNAME_INLINE_LEN)
		{
            kfree(new_name);
			d->d_name.name = d->d_iname;
			strcpy(d->d_iname,name);
            d->d_name.len = new_name_length;
            return 0;
        }
	}
    if(d->d_name.name != NULL && d->d_name.name != d->d_iname)
        kfree(d->d_name.name);
	d->d_name.len  = new_name_length;
	d->d_name.name = new_name;		
    return 0;
}
EXPORT_SYMBOL(dentry_rename);

struct dentry *__d_alloc(struct super_block *sb, const char *name) //创建一个新的空dentry
{
	struct dentry *dentry;
    dentry = kmalloc(sizeof(struct dentry),GFP_KERNEL);
    if(dentry == NULL) return -ENOMEM;
	memset(dentry,0,sizeof(struct dentry));
	if( dentry_rename(dentry,name) < 0){
		kfree(dentry);
		return -ENOMEM;
	}

	INIT_HLIST_HEAD(&dentry->d_hash);
	INIT_HLIST_NODE(&dentry->d_sib);
	spin_lock_init(&dentry->d_lock);
	dentry->d_sb = sb;
	return dentry;
}
EXPORT_SYMBOL(__d_alloc);
/**
 * d_alloc - Allocate and initialize a new dentry for the given parent
 * @parent: pointer to the parent dentry
 * @name: name of the new dentry to allocate
 *
 * This function allocates a new dentry and initializes its fields based on
 * the given parent directory's superblock (`d_sb`). The reference count of
 * the parent dentry is incremented, and the `d_parent` and `d_sb` fields of
 * the new dentry are set accordingly.
 *
 * After allocation, it locks the parent directory to safely update the
 * new dentry's parent and superblock, and then unlocks it.
 *
 * 中文说明：
 * d_alloc - 为给定的父目录分配并初始化一个新的 dentry
 * @parent: 父目录 dentry 指针
 * @name: 新 dentry 的名字
 *
 * 本函数分配一个新的 dentry，并根据给定的父目录的 superblock（`d_sb`）初始化其字段。
 * 新 dentry 的父目录引用计数增加，同时设置 `d_parent` 和 `d_sb` 字段。
 *
 * 在分配后，函数锁住父目录，安全地更新新 dentry 的父目录和 superblock 字段，然后解锁父目录。
 * 
 */
struct dentry *d_alloc(struct dentry * parent, const char *name)
{
	struct dentry *dentry = __d_alloc(parent->d_sb, name);
	if (!dentry)
		return NULL;
	spin_lock(&parent->d_lock);
	dentry->d_parent = dget_dlock(parent);
	dentry->d_sb = dentry->d_parent->d_sb;
	spin_unlock(&parent->d_lock);
	return dentry;
}
EXPORT_SYMBOL(d_alloc);

/**
 * d_getname - Get the name of the given dentry
 * @dentry: pointer to the dentry whose name is to be retrieved
 *
 * This function simply returns the `name` field of the given dentry.
 * It does not perform any validation, so `dentry` must be a valid pointer.
 *
 * 中文说明：
 * d_getname - 获取给定 dentry 的名字
 * @dentry: 要获取名字的 dentry 指针
 *
 * 本函数简单地返回给定 dentry 的 `name` 字段。它不做任何验证，
 * 因此 `dentry` 必须是一个有效的指针。
 */
char * d_getname(struct dentry * dentry){
    if(dentry != NULL)
    return dentry->d_name.name;
}
EXPORT_SYMBOL(d_getname);


/**
 * __d_drop - Release a dentry from memory if no longer needed
 * @dentry: pointer to the dentry to be dropped
 *
 * This function drops a dentry from memory by decreasing the reference
 * count of its parent and freeing the dentry itself. If the dentry name
 * points to an allocated buffer different from the inline buffer, that
 * memory is also freed.
 *
 * This is typically used when the dentry was created temporarily and
 * never added to the global dentry cache, e.g., during failed file
 * creation or removal operations.
 *
 * Notes:
 * - It does not unlink the dentry from parent-child relationships (no list_del).
 * - Safe to call even if dentry was partially initialized.
 *
 * 中文说明：
 * __d_drop - 释放一个不再需要的 dentry
 * @dentry: 要释放的 dentry 指针
 *
 * 此函数用于释放一个内存中的 dentry。它首先减少其父目录的引用计数，
 * 然后判断是否需要释放 dentry 的名字空间（如果使用的是动态分配的内存），
 * 最后释放 dentry 本身。
 *
 * 典型用例是在创建文件失败或删除文件后撤销分配的临时 dentry。
 *
 * 注意事项：
 * - 此函数**不会**从父目录链表中移除 dentry（不调用 list_del）。
 * - 即使 dentry 没有完整初始化也可以安全调用。
 */
void __d_drop(struct dentry *dentry)
{
    dput_dlock(dentry->d_parent); //减少父目录的引用计数
	if(dentry->d_iname == dentry->d_name.name){
		kfree(dentry);
	}
	else{
		kfree(dentry->d_name.name);
		kfree(dentry);
	}
}
EXPORT_SYMBOL(__d_drop);

/**
 * d_delete - Delete a dentry and release its associated inode
 * @dentry: pointer to the dentry to delete
 *
 * This function deletes a dentry by performing two key actions:
 * 1. Calls inode_put() to decrease the reference count of the associated inode.
 * 2. Calls __d_drop() to free the memory of the dentry itself.
 *
 * This should be used when a file or directory is being unlinked
 * and its dentry is no longer needed in memory.
 *
 * 中文说明：
 * d_delete - 删除 dentry 并释放其关联的 inode
 * @dentry: 要删除的 dentry 指针
 *
 * 本函数主要完成两个操作：
 * 1. 调用 inode_put()，减少该 dentry 对应 inode 的引用计数。
 * 2. 调用 __d_drop()，释放 dentry 本身的内存。
 *
 * 适用于文件或目录被删除（unlink）后，从内存中彻底移除 dentry 的场景。
 * 
 * 注意此时它所在处理的dentry已经从vfs中被移除。
 * 如果存在绑定的inode，会自动减少引用计数
 * 
 */
void d_delete(struct dentry * dentry){
    if(dentry == NULL) return;
	inode_put(&dentry->d_inode);
	__d_drop(dentry);
}
EXPORT_SYMBOL(d_delete);


/**
 * __d_add - Add a dentry to a parent's hash list
 * @dentry: pointer to the dentry to add
 * @parent: pointer to the parent dentry
 *
 * This function adds the given dentry to the parent's hash list (`d_hash`)
 * at the head position, establishing a parent-child relationship in the dentry tree.
 *
 * 该函数将给定的 dentry 添加到父目录（parent）的哈希链表 `d_hash` 头部，
 * 在目录树中建立父子关系。
 */
static void __d_add(struct dentry *dentry, struct dentry *parent){
    hlist_add_head(&dentry->d_sib, &parent->d_hash);
}

/**
 * __d_put - Remove a dentry from its parent's hash list
 * @dentry: pointer to the dentry to remove
 *
 * This function removes the given dentry from the parent's hash list (`d_hash`),
 * effectively unlinking it from the parent.
 *
 * 该函数将给定的 dentry 从父目录的哈希链表中移除，解除父子关系。
 */
void __d_put(struct dentry *dentry){
	hlist_del(&dentry->d_sib);
}


/**
 * d_add - Associate a dentry with an inode and add it to the parent's dentry list
 * @dentry: pointer to the dentry to add
 * @inode: pointer to the inode to associate with the dentry
 *
 * This function sets the inode for the given dentry, assigns the superblock
 * from the inode, and then adds the dentry to the parent's hash list (`d_hash`).
 * It is designed to insert the dentry into the directory's dentry cache and link it
 * with the specified inode.
 *
 * 中文说明：
 * d_add - 将 dentry 与 inode 关联并将其添加到父目录的 dentry 列表中
 * @dentry: 要添加的 dentry 指针
 * @inode: 要关联的 inode 指针
 *
 * 本函数为给定的 dentry 设置 inode，并从 inode 中获取其 superblock。
 * 然后，将该 dentry 插入到父目录的哈希链表（d_hash）中。
 * 用于将 dentry 插入目录的 dentry 缓存并与指定的 inode 建立关系。
 */
struct dentry * d_add(struct dentry *dentry, struct inode *inode){
	spin_lock(&dentry->d_parent->d_inode->i_lock);
	dentry->d_inode = inode;
	dentry->d_sb = inode->i_sb;
	__d_add(dentry,dentry->d_parent);
	spin_unlock(&dentry->d_parent->d_inode->i_lock);
	return dentry;
}
EXPORT_SYMBOL(d_add);

/**
 * simple_unlink - Unlink a dentry from its parent and delete it
 * @dir: pointer to the directory inode
 * @dentry: pointer to the dentry to unlink and delete
 *
 * This function unlinks a dentry from its parent directory and then deletes
 * the dentry from the system. It first removes the dentry using __d_put(),
 * and then calls d_delete() to delete the dentry and free its memory.
 *
 * 中文说明：
 * simple_unlink - 从父目录中移除 dentry 并删除它
 * @dir: 目录 inode 指针
 * @dentry: 要移除并删除的 dentry 指针
 *
 * 本函数从父目录中移除 dentry，并调用 d_delete() 删除该 dentry。
 * 它首先通过 __d_put() 移除 dentry，然后调用 d_delete() 释放内存。
 */
int simple_unlink(struct inode *dir, struct dentry *dentry)
{
	if(dentry == NULL)
	return;

	pr_info("simple_unlink\n");
	if(dir != NULL)
	spin_lock(&dir->i_lock);
	__d_put(dentry);
	d_delete(dentry);
	if(dir != NULL)
	spin_unlock(&dir->i_lock);
	return 0;
}
EXPORT_SYMBOL(simple_unlink);



/**
 * d_lookup_dentry - Look up a dentry by name in the parent's hash list
 * @parent: pointer to the parent dentry
 * @name: name of the dentry to look up
 *
 * This function searches for a dentry in the parent's hash list (`d_hash`)
 * by comparing the dentry names. If a match is found, it returns the dentry.
 * Otherwise, it returns NULL if the dentry is not found.
 *
 * 中文说明：
 * d_lookup_dentry - 在父目录的哈希链表中根据名字查找 dentry
 * @parent: 父目录 dentry 指针
 * @name: 要查找的 dentry 名字
 *
 * 本函数在父目录的哈希链表（d_hash）中查找与给定名字匹配的 dentry。
 * 如果找到匹配的 dentry，返回该 dentry；否则返回 NULL，表示未找到。
 * 
 */
struct dentry *d_lookup_dentry(struct dentry *parent, const char *name)
{
    struct dentry *dentry;
    struct hlist_node *node;
    hlist_for_each_entry(dentry, &parent->d_hash, d_sib) {
        if (strcmp(dentry->d_name.name, name) == 0) {
            return dentry;  
        }
    }
    return NULL;
}
EXPORT_SYMBOL(d_lookup_dentry);

