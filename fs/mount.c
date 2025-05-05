#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/cache.h>
#include <linux/list.h> 
#include "fs.h"


extern struct dentry *root_dentry ;
struct list_head superblock_list;


struct super_block * sget(struct file_system_type *type,
    int (*test)(struct super_block *,void *),
    int (*set)(struct super_block *,void *),
    int flags, void *data)
{
    if(type == NULL ) 
        return NULL;
    if( type->mount == NULL)
        return NULL;
    struct dentry * dentry = type->mount(type,0,NULL,data);
    if(IS_ERR(dentry))
        return NULL;
    if(dentry == NULL)
        return NULL;        
    if(dentry->d_sb == NULL) 
        return NULL;        
    return dentry->d_sb;
}

int mount_root_fs(char *file_system_type)
{
    INIT_LIST_HEAD(&superblock_list);
    struct file_system_type * file_system = lookup_fs_type(file_system_type);
    if(file_system == NULL)     return -1;
    struct super_block* test_sb = (file_system->mount(file_system,0,NULL,NULL))->d_sb; 
    if(IS_ERR(test_sb))
        return NULL;


    if(test_sb == NULL)    return -1;
    if( test_sb->s_root == NULL)    return -1;
    strcpy(test_sb->s_sysfs_name,"root");        //实际上这个名字没有任何意义。文件系统不会去查询它
    list_add(&test_sb->s_mounts,&superblock_list); 
    root_dentry = test_sb->s_root;
    root_dentry->d_sb = test_sb;
    root_dentry->d_inode->i_mode = S_IFDIR | 0755;
    dentry_rename(root_dentry,"root");
    pr_info("mount root fs:%s \n",file_system_type);

    return 0;
}

struct  dentry * select_file(char * path , int offset);


int sys_mount(const char __user *dev_name, const char __user *dir_name,
    const char __user *type, unsigned long flags,
    const void __user *data)
{
    struct file_system_type * file_system = lookup_fs_type(type);
    if(file_system == NULL){
        return ERR_PTR(-ENODEV);
    }
    struct  dentry *mount_point = select_file(dir_name , 0);
    if(IS_ERR(mount_point)){
        return ERR_PTR(-ENOENT);
    }
    if((mount_point->d_inode->i_mode & S_IFMT) != S_IFDIR){
        return ERR_PTR(-ENOTDIR);
    }

    spin_lock(&mount_point->d_lock);
    struct super_block* mount_sb = (file_system->mount(file_system,0,dev_name,data))->d_sb; 
    if(IS_ERR(mount_sb))
    return NULL;

    if(mount_sb == NULL)    
        return ERR_PTR(-ENOMEM);
    
    mount_point->d_sb = mount_sb;
    mount_point->d_inode = mount_sb->s_root->d_inode;

    spin_unlock(&mount_point->d_lock);
    pr_info("mount:%s fs:%s to %s \n",dev_name,type,dir_name);
    return 0;
}
