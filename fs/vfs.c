#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/cache.h>
#include <linux/list.h> 
#include <generated/autoconf.h>
#include "fs.h"

struct dentry *root_dentry = NULL;


extern struct super_block * sget(struct file_system_type *type,
    int (*test)(struct super_block *,void *),
    int (*set)(struct super_block *,void *),
    int flags, void *data);

extern int mount_root_fs(char *file_system_type);
extern int sys_mount(const char __user *dev_name, const char __user *dir_name,
    const char __user *type, unsigned long flags,
    const void __user *data);



static struct inode* get_dentry_inode(struct  dentry * dir){
    if(dir == NULL) return NULL;
    if(dir->d_inode != NULL) return dir->d_inode;
    return NULL;
}


static struct dentry* find_dentry(struct dentry* dir,char *dentry_name, unsigned int flags)
{
    struct dentry* son_dentry = d_lookup_dentry(dir,dentry_name); //现在缓存中查询dentry
    if(son_dentry != NULL) {
        return son_dentry;
    }
    son_dentry = d_alloc(dir,dentry_name); 
    if(son_dentry == NULL)
    return ERR_PTR(-ENOMEM);
    struct dentry* son_dentry_ = NULL;
    son_dentry_ =  dir->d_inode->i_op->lookup(dir->d_inode,son_dentry,0); //因为lookup是一定存在的所以不用检测。(不符合规定的会在挂载时被拒绝所以不用在意错误检测)

    if(son_dentry_ == NULL){
        __d_drop(son_dentry); 
        return -ENOENT; 
    }
    return son_dentry_; 
}

static struct dentry* create_dentry(struct dentry* dir,char *name,umode_t i_mode)
{
    struct dentry* next_dentry = find_dentry(dir,name,0);     //如果已有该文件则直接返回
    if(!IS_ERR(next_dentry)){
     pr_info("file already exists: %s\n", name);
     return ERR_PTR(-EEXIST);
    }

    next_dentry = d_alloc(dir,name); //先创建子dentry模板，此时它并没有被真实添加到缓存中
    if(next_dentry == NULL)
    return ERR_PTR(-ENOMEM);

    if(dir->d_inode->i_op->create == NULL){
        __d_drop(next_dentry); //撤销分配没有使用的dentry
     return ERR_PTR(-EROFS);
    }
    int err;
    if((i_mode &  S_IFMT) == S_IFREG)
    err = dir->d_inode->i_op->create(NULL,dir->d_inode,next_dentry,i_mode,0);
    else if((i_mode &  S_IFMT) == S_IFDIR)
    err = dir->d_inode->i_op->mkdir(NULL,dir->d_inode,next_dentry,i_mode);
    else 
    err = -1;

    if(err < 0){
        __d_drop(next_dentry); 
     return ERR_PTR(-ENOMEM);
    }
    return next_dentry;
}

struct  dentry * select_file(char * path , int offset)
{   
    if(path == NULL) 
    return NULL;
    if(path[0] == '/' && path[1] == '\0') 
    return root_dentry;
    int err;    
    int path_numbers = get_dir_number(path);
    char file_name[128];    
    struct dentry * esarch_dentry = root_dentry;
    for(int i = 1 ;i <= path_numbers - offset; i++){
        get_dir_name(path,file_name,i); //不可能错误所以不需要验证
        esarch_dentry = find_dentry(esarch_dentry,file_name,0); 
        if(IS_ERR(esarch_dentry)){
            return esarch_dentry;
        }
    }
    return esarch_dentry;
}

struct  dentry * create_new_dentry(char * path,umode_t i_mode)
{
    struct  dentry * p_dentry;

    p_dentry = select_file(path, 1); //查询子目录
    if(IS_ERR(p_dentry)){
        pr_info("can not find parent dentry%d\n",(int)p_dentry);
        return ERR_PTR(-ENOENT);
    }
    mode_t p_denrty_mode = p_dentry->d_inode->i_mode;
    if((p_denrty_mode & S_IFMT) != S_IFDIR)
    {
        pr_info("parent dentry : %s is not a dir\n",p_dentry->d_name.name);
        return ERR_PTR(-ENOTDIR);
    } 

    char file_name[128];  
    get_dir_name(path,file_name,get_dir_number(path));     
    p_dentry = create_dentry(p_dentry,file_name,i_mode);
    return p_dentry;
}




struct block_device *blkdev_get_by_path(char *path, int flag,void* priv)
{
    struct dentry* dentry = select_file(path,0);
    return devfs_get_blk_dev(dentry->d_inode);
}


struct dentry* mkdir(char *path,umode_t mode){
   if(mode == NULL) mode = 0755;
   return create_new_dentry(path, S_IFDIR | (mode & 0777));
}

struct file *filp_open(const char * path, int flags, umode_t mode)
{
    pr_info("open device %s\n",path);
    struct dentry*file_dentry = select_file(path,0);
    if(IS_ERR(file_dentry))
    {
        if (PTR_ERR(file_dentry) == -ENOENT && (flags & O_CREAT)) 
        {
            pr_info("file not found, try create: %s\n", path);
            file_dentry = create_new_dentry((char *)path, S_IFREG | mode);
            if (IS_ERR(file_dentry)){
                pr_info("can not create file: %s\n", path);
                return (struct file *)file_dentry;
            }
        }
        else {
            return (struct file *)file_dentry;
        }      
    }
    struct file *file = f_get(file_dentry);
    if(IS_ERR(file))
        return ERR_PTR(-ENOMEM);
    file->f_flags = flags;
    file->f_mode = mode;
    int flag = 0;
    spin_lock(&file->f_slock);
    if(file->f_op != NULL){
        spin_lock(&file->f_inode->i_lock);
        if(file->f_op->open != NULL)
        flag = file->f_op->open(file->f_inode,file);
        spin_unlock(&file->f_inode->i_lock);
    }
    spin_unlock(&file->f_slock);
    if(flag < 0){
        f_put(file);
        return NULL;
    }
    
    if(file->f_op->write != NULL)
    file->f_flags |= O_RDONLY;

    return file;
}
EXPORT_SYMBOL(filp_open);

static void print_read_only_message()
{
    pr_info("kernel_write:this file is read only\n");
}
ssize_t kernel_read(struct file *file, void * buf, size_t count, loff_t *ppos)
{
    if(file == NULL) return -1;
    ssize_t size =  -EBADF;
    spin_lock(&file->f_slock);
    if(file->f_op != NULL){
        spin_lock(&file->f_inode->i_lock);
        if(file->f_op->read != NULL)
        size = file->f_op->read(file,buf,count,ppos);
        spin_unlock(&file->f_inode->i_lock);
    }



    spin_unlock(&file->f_slock);
    return size;
}
EXPORT_SYMBOL(kernel_read);

ssize_t kernel_write(struct file *file,const void * buf, size_t count, loff_t *ppos)
{    
    if(file == NULL) return -1;
    if ((file->f_flags & O_ACCMODE) == O_RDONLY) {
        print_read_only_message();
        return -EBADF;  
    }
    ssize_t size;
    spin_lock(&file->f_slock);
    if(file->f_op != NULL){
        spin_lock(&file->f_inode->i_lock);
        ssize_t size = file->f_op->write(file,buf,count,ppos);    
        spin_unlock(&file->f_inode->i_lock);
    }
    spin_unlock(&file->f_slock);
    return size;
}
EXPORT_SYMBOL(kernel_write);


long vfs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if(file == NULL) return -1;
    if ((file->f_flags & O_ACCMODE) == O_RDONLY) {
        print_read_only_message();
        return -EBADF;  // 只读打开，不允许写
    }
    ssize_t size =  -EBADF;
    spin_lock(&file->f_slock);
    if(file->f_op != NULL){
        spin_lock(&file->f_inode->i_lock);
        if(file->f_op->unlocked_ioctl != NULL)
        size = file->f_op->unlocked_ioctl(file,cmd,arg);
        spin_unlock(&file->f_inode->i_lock); 
    }
    spin_unlock(&file->f_slock);
    return size;
}
EXPORT_SYMBOL(vfs_ioctl);

int file_close(struct file *file, fl_owner_t id)
{
    if(file == NULL) return -1;
    spin_lock(&file->f_slock);
    if(file->f_op != NULL){
        spin_lock(&file->f_inode->i_lock);
        if(file->f_op->release != NULL)
         file->f_op->release(file->f_inode,file);
        spin_unlock(&file->f_inode->i_lock);
    }
    spin_unlock(&file->f_slock);
    f_put(file);
}
EXPORT_SYMBOL(file_close);


static int root_fs_init()
{
    int err;
    err = mount_root_fs(CONFIG_ROOT_FILE_SYSTEM_MOUNT);
    if(err < 0){
        pr_info("can not get root fs: devfs\n");
        return -1;
    }
    mkdir("/dev",NULL);
    mkdir("/tmp",NULL);
    mkdir("/mnt",NULL);
    mkdir("/tmp/pipe",NULL);
    sys_mount(NULL,"/dev","devfs",0,NULL);
    sys_mount(NULL,"/tmp/pipe","pipefs",0,NULL);
}
rootfs_initcall(root_fs_init);
