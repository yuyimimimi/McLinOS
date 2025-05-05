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

char * d_getname(struct dentry * dentry){
    if(dentry != NULL)
    return dentry->d_name.name;
}
EXPORT_SYMBOL(d_getname);



void __d_drop(struct dentry *dentry)
{
    dput_dlock(dentry->d_parent);
	if(dentry->d_iname == dentry->d_name.name){
		kfree(dentry);
	}
	else{
		kfree(dentry->d_name.name);
		kfree(dentry);
	}
}
EXPORT_SYMBOL(__d_drop);


void d_delete(struct dentry * dentry){
    if(dentry == NULL) return;
	inode_put(&dentry->d_inode);
	__d_drop(dentry);
}
EXPORT_SYMBOL(d_delete);

static void __d_add(struct dentry *dentry, struct dentry *parent){
    hlist_add_head(&dentry->d_sib, &parent->d_hash);
}
void __d_put(struct dentry *dentry){
	hlist_del(&dentry->d_sib);
}

struct dentry * d_add(struct dentry *dentry, struct inode *inode){
	spin_lock(&dentry->d_parent->d_inode->i_lock);
	dentry->d_inode = inode;
	dentry->d_sb = inode->i_sb;
	__d_add(dentry,dentry->d_parent);
	spin_unlock(&dentry->d_parent->d_inode->i_lock);
	return dentry;
}
EXPORT_SYMBOL(d_add);



int simple_unlink(struct inode *dir, struct dentry *dentry)
{
	if(dir != NULL)
	spin_lock(&dir->i_lock);
	__d_put(dentry);
	d_delete(dentry);
	if(dir != NULL)
	spin_unlock(&dir->i_lock);
}
EXPORT_SYMBOL(simple_unlink);


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