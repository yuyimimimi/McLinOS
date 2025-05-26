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
    struct dentry* son_dentry = d_lookup_dentry(dir,dentry_name);
    if(son_dentry != NULL) {
        //pr_info("find from chache\n");
        return son_dentry;
    }
    son_dentry = d_alloc(dir,dentry_name); 
    if(son_dentry == NULL)
    return ERR_PTR(-ENOMEM);
    struct dentry* son_dentry_ = NULL;
    son_dentry_ =  dir->d_inode->i_op->lookup(dir->d_inode,son_dentry,0); 
    if(son_dentry_ == NULL || IS_ERR(son_dentry_)){
       
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
    if((i_mode & S_IFMT) == S_IFREG)
    {
       // pr_info("create file \n");
        err = dir->d_inode->i_op->create(NULL,dir->d_inode,next_dentry,i_mode,0);
    }
    else if((i_mode &  S_IFMT) == S_IFDIR)
    {
       // pr_info("create dir \n");
        err = dir->d_inode->i_op->mkdir(NULL,dir->d_inode,next_dentry,i_mode);
    }
    else 
    err = -1;

    if(err < 0){
        __d_drop(next_dentry); 
     return ERR_PTR(-ENOMEM);
    }
    return next_dentry;
}


static int __remove_dentry(struct dentry* dentry)    
{
    if(dentry == NULL){                            
        return -1;                                    
    }
    if(dentry->d_sb->s_root == dentry){
        pr_warn("can not remove mount point\n");
        return -1;
    }
    umode_t i_mode = dentry->d_inode->i_mode;      
    if((i_mode &  S_IFMT) == S_IFCHR || (i_mode &  S_IFMT) == S_IFBLK  ){
        return -1;
    }
    else if((i_mode &  S_IFDIR) == S_IFCHR)
    {
        dentry->d_inode->i_op->rmdir(dentry->d_parent->d_inode,dentry);
    }
    else{
        dentry->d_inode->i_op->unlink(dentry->d_parent->d_inode,dentry);
        simple_unlink(dentry->d_parent->d_inode,dentry);
    }
    return 0;
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

/**
 * 根据完整路径创建所有必要的目录和最终文件
 * @param path 完整路径(如"/a/b/c/file.txt")
 * @param mode 最终文件的模式
 * @return 成功返回目标dentry，失败返回错误指针
 */
static struct dentry *create_path_and_file(const char *path, umode_t mode)
{
    struct dentry *parent = root_dentry;
    struct dentry *child = NULL;
    char component[NAME_MAX];
    const char *start = path;
    const char *end;
    int err;
    if (*start != '/') 
    return NULL;

    if (*start == '/') start++;
    while ((end = strchr(start, '/'))) {
        size_t len = end - start;
        if (len >= sizeof(component)) {
            return ERR_PTR(-ENAMETOOLONG);
        }
        strncpy(component, start, len);
        component[len] = '\0';
        child = find_dentry(parent, component, 0);
        if (IS_ERR(child)) {
            child = create_dentry(parent, component, S_IFDIR | 0755);
            if (IS_ERR(child)) {
                pr_err("Failed to create directory '%s': %ld\n",
                      component, PTR_ERR(child));
                return child;
            }
        }
        if (!S_ISDIR(child->d_inode->i_mode)) {
            return ERR_PTR(-ENOTDIR);
        }
        parent = child;
        start = end + 1;
    }
    if (*start != '\0') {
        child = create_dentry(parent, start, mode);
        if (IS_ERR(child)) {
            pr_err("Failed to create file '%s': %ld\n",
                  start, PTR_ERR(child));
            return child;
        }
        return child;
    }
    return parent;
}



struct  dentry * create_new_dentry(char * path,umode_t i_mode)
{
    struct  dentry * p_dentry;

    p_dentry = select_file(path, 1); //查询子目录
    if(IS_ERR(p_dentry)){
        //pr_info("can not find parent dentry%d\n",(int)p_dentry);
        return ERR_PTR(-ENOENT);
    }
    mode_t p_denrty_mode = p_dentry->d_inode->i_mode;
    if((p_denrty_mode & S_IFMT) != S_IFDIR)
    {
        //pr_info("parent dentry : %s is not a dir\n",p_dentry->d_name.name);
        return ERR_PTR(-ENOTDIR);
    } 
    char file_name[128];  
    get_dir_name(path,file_name,get_dir_number(path));     
    p_dentry = create_dentry(p_dentry,file_name,i_mode);
    return p_dentry;
}

int remove_dentry(char* path)
{
    struct  dentry * dentry = select_file(path,0);
    if(!IS_ERR(dentry)){
        __remove_dentry(dentry);
        return 0;
    }
    return -1;
}
EXPORT_SYMBOL(remove_dentry);

struct block_device *blkdev_get_by_path(char *path, int flag,void* priv)
{
    struct dentry* dentry = select_file(path,0);
    return devfs_get_blk_dev(dentry->d_inode);
}
EXPORT_SYMBOL(blkdev_get_by_path);

struct dentry* mkdir(char *path,umode_t mode){
   if(mode == NULL) mode = 0755;
   return create_new_dentry(path, S_IFDIR | (mode & 0777));
}
EXPORT_SYMBOL(mkdir);


umode_t get_file_mode(char *path){
    struct dentry*file_dentry = select_file(path,0);
    if(IS_ERR(file_dentry)){
        return 0;
    }
    return file_dentry->d_inode->i_mode;
}
void *print_mode(char *path)
{
    umode_t mode = get_file_mode(path);
    char data[10];
    data[0] = (S_ISDIR(mode)) ? 'd' : 
    (S_ISCHR(mode)) ? 'c' : 
    (S_ISBLK(mode)) ? 'b' : 
    (S_ISFIFO(mode)) ? 'p' : 
    (S_ISLNK(mode)) ? 'l' : 
    (S_ISSOCK(mode)) ? 's' : '-';
    data[1] = (mode & S_IRUSR) ? 'r' : '-';
    data[2] = (mode & S_IWUSR) ? 'w' : '-';
    data[3] = (mode & S_IXUSR) ? 'x' : '-';
    data[4] = (mode & S_IRGRP) ? 'r' : '-';
    data[5] = (mode & S_IWGRP) ? 'w' : '-';
    data[6] = (mode & S_IXGRP) ? 'x' : '-';
    data[7] = (mode & S_IROTH) ? 'r' : '-';
    data[8] = (mode & S_IWOTH) ? 'w' : '-';
    data[9] = (mode & S_IXOTH) ? 'x' : '-';
    printk(KERN_INFO "File mode: %s\n", data);
    return NULL;
}

struct file *filp_open(const char * path, int flags, umode_t mode)
{
    printk(KERN_DEBUG "open file %s\n",path);
    struct dentry*file_dentry = select_file(path,0);
    if(IS_ERR(file_dentry))
    {
        pr_info("file not found\n");
        if (PTR_ERR(file_dentry) == -ENOENT && (flags & O_CREAT)) 
        {
            pr_info("try create: %s\n", path);
            file_dentry = create_new_dentry((char *)path, S_IFREG | mode);
            if (IS_ERR(file_dentry)){
                pr_info("can not create file: %s\n", path);
                return (struct file *)file_dentry;
            }
        }
        else {
            return -ENOENT;
        }      
    }
    struct file *file = f_get(file_dentry);
    if(IS_ERR(file))
        return ERR_PTR(-ENOMEM);

    if ((file->f_inode->i_mode & S_IFMT) == S_IFDIR) {
        file->f_pos = 0;
    }

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
    file->f_inode->i_atime_nsec  = jiffies/HZ;
    file->f_inode->i_atime_nsec  = jiffies%HZ;
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
    ssize_t size = -EBADF;
    if(file == NULL) return -1;
    if ((file->f_flags & O_ACCMODE) == O_RDONLY) {
        print_read_only_message();
        return -EBADF;  
    }
    spin_lock(&file->f_slock);
    if(file->f_op != NULL){
        spin_lock(&file->f_inode->i_lock);
        size = file->f_op->write(file,buf,count,ppos);   
        file->f_inode->i_mtime_sec   = jiffies/HZ;
        file->f_inode->i_mtime_nsec  = jiffies%HZ; 
        spin_unlock(&file->f_inode->i_lock);
    }
    spin_unlock(&file->f_slock);
    return size;
}
EXPORT_SYMBOL(kernel_write);


long vfs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    ssize_t size =  -EBADF;
    if(file == NULL) return -1;
    if ((file->f_flags & O_ACCMODE) == O_RDONLY) {
        print_read_only_message();
        return -EBADF;  // 只读打开，不允许写
    }
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

int filp_close(struct file *file, fl_owner_t id)
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
    return 0;
}
EXPORT_SYMBOL(filp_close);




#include <linux/initramfs.h>
static int mountfile_to_initramfs(__file_data *file_data)
{
    struct dentry *file = create_path_and_file(file_data->path, S_IFREG | 0755);
    if(file == NULL){
        return -1;
    }
    initram_fs_mount_file(file->d_inode,file_data);
    return 0;
}
static void mount_all_file(void){
    for (__file_data *file_head = __export_file_start; file_head != __export_file_end; file_head++) {
        mountfile_to_initramfs(file_head);
    }
}


static int root_fs_init()
{
    int err;
    err = mount_root_fs(CONFIG_ROOT_FILE_SYSTEM_MOUNT);
    if(err < 0){
        pr_info("can not get root fs: devfs\n");
        return -1;
    }
    
    mount_all_file();
    
    mkdir("/dev",0755);
    mkdir("/tmp",0755);
    mkdir("/mnt",0755);
    mkdir("/sys",0755);
    mkdir("/tmp/pipe",NULL);
    sys_mount(NULL,"/dev","devfs",0,NULL);
    sys_mount(NULL,"/tmp/pipe","pipefs",0,NULL);
    sys_mount(NULL,"/sys","sysfs",0,NULL);
    return 0;
}

rootfs_initcall(root_fs_init);
