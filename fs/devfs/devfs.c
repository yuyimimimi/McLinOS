#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/devfs.h>



#define MAGIC 12345678
struct devfs_inode {
    int                     magic;
    uint32_t                i_mode;
    uint32_t                major;
    atomic_t                dentry_count;          //引用计数
    struct file_operations *i_fop;                
    struct list_head        list_node;             // 挂在superblock的inode链表
    struct list_head        dentry_list_head;      // 如果这是目录项，这里存储inode下的所有dentry链表头
    spinlock_t              lock;
    
    struct block_device       *bdev;

    struct devfs_superblock *sb;
};
struct devfs_dentry {
    int                    magic;
    char *name;                             // 名字
    struct devfs_inode *target_inode;       // 该dentry指向的目标inode
    struct list_head list_node;             // 挂载到父目录inode中的listhead中
};
struct devfs_superblock {
    int                    magic;
    spinlock_t             lock;
    struct devfs_inode *rootinode;
    struct list_head inode_list_head;       // 该superblock下的inode链表头
};

int devfs_mount_device(struct inode *inode,unsigned int major,struct file_operations *fop){
    struct devfs_inode *devfs_inode = inode->i_private;
    if(devfs_inode->magic != MAGIC) return -1;
    devfs_inode->i_fop = fop;
    devfs_inode->major = major;
    devfs_inode->i_mode = S_IFCHR | (devfs_inode->i_mode & 0777);
    inode->i_fop = fop;
    return 0;
}

int devfs_mount_blk_device(struct inode *inode,struct block_device* bdev,uint32_t major ){
    struct devfs_inode *devfs_inode = inode->i_private;
    if(devfs_inode->magic != MAGIC) return -1;
    devfs_inode->major = major;
    devfs_inode->bdev = bdev;
    devfs_inode->i_mode = S_IFBLK | (devfs_inode->i_mode & 0777);
    return 0;
}
struct block_device* devfs_get_blk_dev(struct inode *inode)
{
    struct devfs_inode *devfs_inode = inode->i_private;
    if((devfs_inode->i_mode & S_IFMT) == S_IFBLK) 
    return devfs_inode->bdev;
    else 
    return NULL;
}



static int devfs_open(struct inode *inode, struct file *file){
    early_printk("devfs test open\n");
    return 0;
}
static int devfs_release(struct inode *inode, struct file *file){
    early_printk("devfs test close\n");
    return 0;
}
static int devfs_read(struct file *file, char __user * data, size_t size, loff_t *offset){
    early_printk("devfs test read\n");
    return 0;
}
static int devfs_write(struct file *file, char __user * data, size_t size, loff_t *offset){
    early_printk("devfs test write\n");
    return 0;
}


static struct file_operations devfs_file_fops = {
    .owner = THIS_MODULE,
    .open = devfs_open,
    .release = devfs_release,
    .read  = devfs_read,
    .write = devfs_write
};

static struct devfs_inode *devfs_create_empty_inode(struct devfs_superblock *sb,struct file_operations *fop, uint32_t major) //普通地创建一个inode
{
    if(fop == NULL) return NULL;
    struct devfs_inode *dev_inode = kmalloc(sizeof(struct devfs_inode),GFP_KERNEL);
    if(dev_inode == NULL) return NULL;
    dev_inode->magic = MAGIC;
    dev_inode->major = major;
    dev_inode->i_fop = fop;
    dev_inode->sb    = sb;
    dev_inode->i_mode = S_IFREG | 0755;
    atomic_set(&dev_inode->dentry_count,0);
    spin_lock_init(&dev_inode->lock);
    INIT_LIST_HEAD(&dev_inode->list_node);
    INIT_LIST_HEAD(&dev_inode->dentry_list_head);
    
    spin_lock(&sb->lock);
    list_add(&dev_inode->list_node,&sb->inode_list_head);
    spin_unlock(&sb->lock);
    
    return dev_inode;
}
static void devfs_inode_put(struct devfs_inode *node){
    if(node != NULL)kfree(node);
}
static struct devfs_dentry* devfs_create_dentry(struct devfs_inode* parent_dentry_inode,struct devfs_inode* target_inode,char *name) //创建硬连接
{
    if(parent_dentry_inode == NULL || name == NULL || target_inode == NULL) return NULL;

    struct devfs_dentry* new_dentry =  kmalloc(sizeof(struct devfs_dentry),GFP_KERNEL);
    if(new_dentry == NULL)return NULL;
    new_dentry->magic = MAGIC;
    new_dentry->name = NULL;
    new_dentry->name = kmalloc(strlen(name) + 1,GFP_KERNEL);
    if(new_dentry->name  == NULL) {
        kfree(new_dentry);
        return NULL;
    }
    strcpy(new_dentry->name,name);
    new_dentry->target_inode = target_inode;
    INIT_LIST_HEAD(&new_dentry->list_node);
    

    atomic_inc (&target_inode->dentry_count);      //增加引用计数
    
    
    spin_lock   (&parent_dentry_inode->lock);
    list_add    (&new_dentry->list_node,&parent_dentry_inode->dentry_list_head);
    spin_unlock(&parent_dentry_inode->lock);
    return new_dentry;
}
static struct devfs_dentry* devfs_create_inode( struct devfs_superblock *sb,struct devfs_inode* parent_dentry_inode,struct file_operations* fop,uint32_t major,char *name)
{
    struct devfs_inode * new_inode = devfs_create_empty_inode(sb,fop,major);
    if(new_inode == NULL){
        return NULL;
    }   
    struct devfs_dentry* new_dentry = devfs_create_dentry(parent_dentry_inode,new_inode,name); //创建硬连接
    if(new_dentry == NULL){
        spin_lock(&sb->lock);
        list_del(&new_inode->list_node);
        spin_unlock(&sb->lock);
        devfs_inode_put(new_inode);
        return NULL;
    }
    return new_dentry;
}
static struct devfs_dentry* devfs_create_file(struct devfs_superblock *sb,struct devfs_inode* parent_dentry_inode,struct file_operations* fop,uint32_t major,char *name)
{
    return devfs_create_inode(sb,parent_dentry_inode,fop,major,name);
}

static struct devfs_dentry* devfs_create_dir(struct devfs_superblock *sb,struct devfs_inode* parent_dentry_inode,char *name)
{
    struct devfs_dentry* dentry =  devfs_create_inode(sb,parent_dentry_inode,&devfs_file_fops,0,name);
    if(dentry == NULL) return NULL;
    dentry->target_inode->i_mode = S_IFDIR | 0755;
    return dentry;
}
static void dentry_put(struct devfs_superblock *sb,struct devfs_dentry* dentry)
{
    if(dentry == NULL) return;
    if(dentry->target_inode != NULL)
    {
        spin_lock(&sb->lock);
        if (atomic_dec_and_test(&dentry->target_inode->dentry_count)) //检测inode引用计数是否为0，如果为0则删除
        {
            list_del(&dentry->target_inode->list_node);    
            devfs_inode_put(dentry->target_inode);
        }
        spin_unlock(&sb->lock);
    }
    kfree(dentry->name);
    kfree(dentry);
}
static void devfs_delete_dentry(struct devfs_superblock *sb,struct devfs_inode* parent_dentry_inode,struct devfs_dentry* delete_dentry){
    if(parent_dentry_inode != NULL) spin_lock(&parent_dentry_inode->lock);
    
    if ((delete_dentry->target_inode->i_mode & S_IFMT) == S_IFDIR ){         //如果是目录项需要验证是否存在子项
        if(!list_empty(&delete_dentry->target_inode->dentry_list_head)){      //如果目录下的inode存储的dentry结点不为空，则拒绝删除
            if(parent_dentry_inode != NULL) 
                spin_unlock(&parent_dentry_inode->lock);
            return;
        }
    }
    list_del(&delete_dentry->list_node);                                     //将dentry从父目录中移除
    if(parent_dentry_inode != NULL) spin_unlock(&parent_dentry_inode->lock);
    dentry_put(sb,delete_dentry);
}
static struct devfs_superblock* devfs_superblock_create(void) //创建superblock
{
    struct devfs_superblock*d_sb = kmalloc(sizeof(struct devfs_superblock),GFP_KERNEL);
    if(d_sb == NULL){
        return NULL;        
    }
    d_sb->magic = MAGIC;
    spin_lock_init(&d_sb->lock);
    INIT_LIST_HEAD(&d_sb->inode_list_head);
    struct devfs_inode* root_inode = devfs_create_empty_inode(d_sb ,&devfs_file_fops,0);
    if(root_inode == NULL){
        kfree(d_sb);
        return NULL;
    }
    root_inode->i_mode = S_IFDIR | 0755;
    d_sb->rootinode = root_inode;
    root_inode->i_fop = &devfs_file_fops;
    atomic_inc (&root_inode->dentry_count); 
    INIT_LIST_HEAD(&root_inode->dentry_list_head);
    return d_sb;
} 
static void devfs_superblock_destory(struct devfs_superblock* sb){
    if(sb!= NULL) {
        struct devfs_inode * root_inode = sb->rootinode;
        if(!root_inode){
            kfree(root_inode);
        }
        kfree(sb);
    }
}

static struct devfs_dentry* devfs_lookup(struct devfs_inode* dentry_inode,char *name)
{
    if(dentry_inode == NULL) return NULL;
    if ( (dentry_inode->i_mode & S_IFMT) != S_IFDIR ) return NULL;
    struct devfs_dentry* pos;
    spin_lock(&dentry_inode->lock);
    list_for_each_entry(pos, &dentry_inode->dentry_list_head,list_node ) {
        if (strcmp(pos->name, name) == 0) {
            spin_unlock(&dentry_inode->lock);
            return pos;
        }
    }
    spin_unlock(&dentry_inode->lock);
    return NULL;
}









static struct super_operations devfs_super_operation;
static struct dentry_operations devfs_dentry_operation;
static struct inode_operations devfs_inode_operation;

static void devfs_kill_sb (struct super_block * sb);
static struct inode *devfs_alloc_inode(struct super_block *sb);
static void  devfs_destroy_inode(struct inode * node); //撤销devfs_alloc_inode所做的一切
static int devfs_statfs (struct dentry * dentry, struct kstatfs *star);
static struct dentry * devfs_get_dentry(struct dentry * vdentry, struct devfs_dentry *dentry);
static struct super_block *devfs_get_vfs_superblock(struct devfs_superblock *sb);
static struct inode *  devfs_get_inode(struct devfs_inode *,struct super_block*);
static struct dentry *devfs_mount(struct file_system_type *fs_type,
    int flags,
    const char *dev_name,
    void *data);

static struct file_system_type fs_type = {
    .name     = "devfs",
    .fs_flags = 0,
    .init_fs_context = NULL,
    .kill_sb  = devfs_kill_sb,
    .mount    = devfs_mount,
    .owner    = THIS_MODULE,
};

static struct dentry *devfs_mount(struct file_system_type *fs_type,
    int flags,
    const char *dev_name,
    void *data)
{
    struct devfs_superblock *fs_superblock= devfs_superblock_create();
    if (fs_superblock == NULL){
        return ERR_PTR(-ENOMEM);        
    }
    struct dentry * root = __d_alloc(NULL,"root");
    if(root == NULL){
         devfs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);   
    }


    struct devfs_dentry* dentry = kmalloc(sizeof(struct devfs_dentry),GFP_KERNEL);
    if(dentry == NULL){
       
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    }
    dentry->name = NULL;
    dentry->target_inode = fs_superblock->rootinode;
    dentry->magic =MAGIC;
    root->d_fsdata = dentry;
    struct inode * root_inode = devfs_get_inode(dentry->target_inode,NULL);
    if(root_inode == NULL){
        kfree(dentry);
        devfs_superblock_destory(fs_superblock);
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    } 
    root->d_inode = root_inode;

    struct super_block *sb =devfs_get_vfs_superblock(fs_superblock);
    if(sb == NULL){
        kfree(root);
        kfree(dentry);
        inode_put(root_inode);
        devfs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);       
    }
    sb->s_root = root;
    root->d_sb = sb;

    return root;
}



static struct super_operations devfs_super_operation = {
    .alloc_inode   = devfs_alloc_inode,
    .destroy_inode = devfs_destroy_inode, 
    .free_inode    = NULL,         
    .dirty_inode   = NULL,
    .write_inode   = NULL,
    .drop_inode    = NULL,
    .put_super     = devfs_kill_sb,
    .sync_fs       = NULL,
    .statfs        = devfs_statfs,
};

static struct super_block *devfs_get_vfs_superblock(struct devfs_superblock *sb) //将文件系统的superblock映射到vfs
{
    if(sb == NULL) return NULL;
    struct super_block * vsb  = alloc_super(NULL);                        //分配了一个superblock(注意释放)
    if(vsb == NULL) return NULL;
    vsb->s_fs_info =   sb;
    vsb->s_d_op    =  &devfs_dentry_operation;
    vsb->s_op      =  &devfs_super_operation;    
    return vsb;
}


static void devfs_kill_sb (struct super_block * sb){                          //释放文件系统实例，所以并不会删除superblock本身只是删除映射 
    put_super(sb);
}

static struct inode *  devfs_get_inode(struct devfs_inode * dev_node,struct super_block* sb )  //通过该文件系统的inode转化为vfs的标准inode(不修改自身inode，需要分配inode)
{
    if(dev_node == NULL) return NULL;
    struct inode*inode  =  new_inode(sb); //此处动态创建了一个inode,需要后续释放
    if(inode == NULL) return NULL;  
    inode->i_mode    = dev_node->i_mode;
    inode->i_fop     = dev_node->i_fop;
    inode->i_op      = &devfs_inode_operation;
    inode->i_private = dev_node;
    return inode;
}

static struct inode *devfs_alloc_inode(struct super_block *sb) //创建一个inode，同时创建inode和文件系统的inode。但是没有硬链接
{
    struct devfs_superblock *supb = sb->s_fs_info;
    if(supb->magic != MAGIC) return NULL;
     struct devfs_inode * inode = devfs_create_empty_inode(supb,&devfs_file_fops,10); //文件系统分配一个inode
     if(inode == NULL) return NULL;
     struct inode *out_inode = devfs_get_inode(inode,sb);
     if(out_inode == NULL){
        kfree(inode);
        return NULL;
     }
     return out_inode;
}



static void  devfs_destroy_inode(struct inode * node) //撤销devfs_alloc_inode所做的一切
{
    if(node == NULL) return;
    if(node->i_private != NULL) {
        struct devfs_inode * dev_node = node->i_private;
        list_del(&dev_node->list_node);
        kfree(dev_node);        
    }
}

static int devfs_statfs (struct dentry * dentry, struct kstatfs *star){
    struct devfs_dentry * devfs_dentry = dentry->d_fsdata;
    struct devfs_inode * devfs_node = devfs_dentry->target_inode;
    star->f_type = MAGIC;
    star->f_bsize  = 0;
    star->f_blocks = 1;
    star->f_bfree  = 0;
    star->f_bavail = 0;
    star->f_files  = 128;
    star->f_ffree  = 127;
    return 0;
}



static int devfs_create(struct mnt_idmap * map, struct inode * dir,struct dentry * dentry,umode_t mode, bool bool);
static struct dentry *devfs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags);
static int devfs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry);
static int devfs_unlink(struct inode *dir, struct dentry *dentry);
static int devfs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode);
static int devfs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr);
static int  devfs_getattr (struct mnt_idmap *dmp, const struct path * path,struct kstat *stat, u32 u, unsigned int i);
static int devfs_rmdir(struct inode *	inode  ,struct dentry *dentry);

static struct inode_operations devfs_inode_operation = {
    .create = devfs_create,
    .lookup = devfs_vfs_lookup,
    .link   = devfs_link,
    .unlink = devfs_unlink,
    .mkdir  = devfs_mkdir,
    .rmdir  = devfs_rmdir,
    .setattr = devfs_setattr,
    .getattr = devfs_getattr
};

static struct dentry * devfs_get_dentry(struct dentry * vdentry, struct devfs_dentry *dentry) //通过dentry获得标准的dentry。但是此处的dentry不是自己动态分配的
{
    if(dentry == NULL || vdentry == NULL) return NULL;
    vdentry->d_fsdata = dentry;
    return vdentry;
}

static int devfs_create(struct mnt_idmap * map, struct inode * dir,struct dentry * dentry,umode_t mode, bool bool)
{
    char *file_name = d_getname(dentry);

    struct devfs_inode *dinode = dir->i_private;
    struct devfs_dentry* file = devfs_lookup(dinode,file_name);
    if(file == NULL){
        file =  devfs_create_file(dinode->sb,dinode,&devfs_file_fops,10,file_name);
    }
    if (file == NULL){                               //确定文件是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    if( devfs_get_dentry(dentry,file) == NULL){     //构建dentry    
     return -1;
    }  

    struct inode * new_file_inode = devfs_get_inode(dinode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); //通知文件系统加入缓存，并进行资源绑定
   return 0;
} 

static struct dentry *devfs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags){
    
    struct devfs_inode *dir_inode = (struct devfs_inode *)dir->i_private; //获取文件系统本身的inode
    char *name = dentry->d_name.name;                           
    struct devfs_dentry *pos;    
    pos = devfs_lookup(dir_inode,name);       //查询获得dentry
    if(pos == NULL) return NULL;

    if( devfs_get_dentry(dentry,pos) == NULL)
        return NULL;

    struct inode * find_inode = devfs_get_inode(pos->target_inode,dir->i_sb);
    if(find_inode == NULL)
        return NULL;
    
    return d_add(dentry,find_inode);
    return NULL;
}


static int devfs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry)
{
    struct devfs_inode *old_inode = (struct devfs_inode *)old_dentry->d_inode->i_private;
    struct devfs_inode *dir_inode = (struct devfs_inode *)dir->i_private;
    struct devfs_dentry *new_dentry = devfs_create_dentry(dir_inode, old_inode, dentry->d_name.name);
    if (new_dentry == NULL) {
        return -ENOMEM;
    }
    devfs_get_dentry(dentry,new_dentry);
    dentry->d_sb = dir->i_sb;
    d_add(dentry,dentry->d_inode);
    return 0;
}

static int devfs_unlink(struct inode *dir, struct dentry *dentry)
{
    struct devfs_inode *parent_inode = (struct devfs_inode *)dir->i_private;
    struct devfs_dentry *delete_dentry = (struct devfs_dentry *)dentry->d_fsdata;
    devfs_delete_dentry(parent_inode->sb, parent_inode, delete_dentry);
    return 0;
}


static int devfs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    char *file_name = d_getname(dentry);

    struct devfs_inode *dinode = dir->i_private;
    struct devfs_dentry* file = devfs_lookup(dinode,file_name);
    if(file == NULL){
        file =  devfs_create_dir(dinode->sb,dinode,file_name);
    }
    if (file == NULL){                               //确定文件是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    
    if( devfs_get_dentry(dentry,file) == NULL){     //构建dentry    
     return -1;
    }  
    struct inode * new_file_inode = devfs_get_inode(dinode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); //通知文件系统加入缓存，并进行资源绑定
   return 0;
}


static int devfs_release_dentry(struct dentry *dentry){
    struct devfs_dentry *fs_dentry = dentry->d_fsdata;
    dentry_put(fs_dentry->target_inode->sb, fs_dentry);
}
static int devfs_rmdir(struct inode *	dir  ,struct dentry *dentry){
    devfs_release_dentry(dentry);
    return simple_unlink(dir,dentry);
}

static int devfs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr)
{
    struct devfs_dentry* file = dentry->d_fsdata;
    file->target_inode->i_mode = (iattr->ia_mode & S_IFMT) | iattr->ia_mode;
    return 0;
}
static int  devfs_getattr (struct mnt_idmap *dmp, const struct path * path,struct kstat *stat, u32 u, unsigned int i)
{
    struct dentry *d = path->dentry;
    struct devfs_dentry* file = d->d_fsdata;
    struct devfs_inode *inode;

    if (!file || !(inode = file->target_inode))
    return -ENOENT;
    stat->mode = inode->i_mode;
    stat->dev  = MKDEV(inode->major, 0);
    stat->ino  = (unsigned long)inode;  
    stat->nlink = atomic_read(&inode->dentry_count);
    stat->size  = 0; 
    stat->atime.tv_nsec = jiffies/HZ;
    stat->atime.tv_sec  = jiffies%HZ;
    stat->mtime = stat->atime;
    stat->ctime = stat->atime;
    stat->blksize = PAGE_SIZE;
    stat->blocks  = 0;
    return 0;
}

static struct dentry_operations devfs_dentry_operation = {
    .d_release = devfs_release_dentry,
};

static int __init devfs_ops_init(void){
    register_filesystem(&fs_type);
    return 0;
}

fs_initcall(devfs_ops_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("你");
MODULE_DESCRIPTION("简易设备文件系统 devfs 测试");







