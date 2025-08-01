#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>


#define MAGIC 12345678

struct sys_fs_inode {
    int                     magic;
    uint32_t                i_mode;
    uint32_t                major;
    atomic_t                dentry_count;        
    struct file_operations *i_fop;                
    struct list_head        list_node;            
    struct list_head        dentry_list_head;     
    spinlock_t              lock;
    struct block_sys_ice       *bsys_;
    struct sys_fs_superblock *sb;
    struct kobject * kobject;
};

struct sys_fs_dentry {
    int                    magic;
    char *name;                             
    struct sys_fs_inode *target_inode;      
    struct list_head list_node;         
};
struct sys_fs_superblock {
    int                    magic;
    spinlock_t             lock;
    struct sys_fs_inode *rootinode;
    struct list_head inode_list_head;      
};

int sysfs_mount_kobject(struct inode *inode, struct kobject * kobject)
{
    struct sys_fs_inode *sysfs_inode = inode->i_private;
    if(sysfs_inode->magic != MAGIC) return -1;
    sysfs_inode->i_mode = S_ISVTX | (sysfs_inode->i_mode & 0777);
    sysfs_inode->kobject = kobject;
    return 0;
}

static int sys_fs_open(struct inode *inode, struct file *file){
    early_printk("sys_fs test open\n");
    return 0;
}
static int sys_fs_release(struct inode *inode, struct file *file){
    early_printk("sys_fs test close\n");
    return 0;
}
static int sys_fs_read(struct file *file, char __user * data, size_t size, loff_t *offset){
    early_printk("sys_fs test read\n");
    return 0;
}
static int sys_fs_write(struct file *file, char __user * data, size_t size, loff_t *offset){
    early_printk("sys_fs test write\n");
    return 0;
}

static struct file_operations sys_fs_file_fops = {
    .owner = THIS_MODULE,
    .open = sys_fs_open,
    .release = sys_fs_release,
    .read  = sys_fs_read,
    .write = sys_fs_write
};

static struct sys_fs_inode *sys_fs_create_empty_inode(struct sys_fs_superblock *sb,struct file_operations *fop, uint32_t major) //普通地创建一个inode
{
    if(fop == NULL) return NULL;
    struct sys_fs_inode *sys__inode = kmalloc(sizeof(struct sys_fs_inode),GFP_KERNEL);
    if(sys__inode == NULL) return NULL;
    sys__inode->magic = MAGIC;
    sys__inode->major = major;
    sys__inode->i_fop = fop;
    sys__inode->sb    = sb;
    sys__inode->i_mode = S_IFREG | 0755;
    atomic_set(&sys__inode->dentry_count,0);
    spin_lock_init(&sys__inode->lock);
    INIT_LIST_HEAD(&sys__inode->list_node);
    INIT_LIST_HEAD(&sys__inode->dentry_list_head);
    
    spin_lock(&sb->lock);
    list_add(&sys__inode->list_node,&sb->inode_list_head);
    spin_unlock(&sb->lock);
    
    return sys__inode;
}
static void sys_fs_inode_put(struct sys_fs_inode *node){
    if(node != NULL)kfree(node);
}
static struct sys_fs_dentry* sys_fs_create_dentry(struct sys_fs_inode* parent_dentry_inode,struct sys_fs_inode* target_inode,char *name) //创建硬连接
{
    if(parent_dentry_inode == NULL || name == NULL || target_inode == NULL) return NULL;

    struct sys_fs_dentry* new_dentry =  kmalloc(sizeof(struct sys_fs_dentry),GFP_KERNEL);
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
static struct sys_fs_dentry* sys_fs_create_inode( struct sys_fs_superblock *sb,struct sys_fs_inode* parent_dentry_inode,struct file_operations* fop,uint32_t major,char *name)
{
    struct sys_fs_inode * new_inode = sys_fs_create_empty_inode(sb,fop,major);
    if(new_inode == NULL){
        return NULL;
    }   
    struct sys_fs_dentry* new_dentry = sys_fs_create_dentry(parent_dentry_inode,new_inode,name); //创建硬连接
    if(new_dentry == NULL){
        spin_lock(&sb->lock);
        list_del(&new_inode->list_node);
        spin_unlock(&sb->lock);
        sys_fs_inode_put(new_inode);
        return NULL;
    }
    return new_dentry;
}
static struct sys_fs_dentry* sys_fs_create_file(struct sys_fs_superblock *sb,struct sys_fs_inode* parent_dentry_inode,struct file_operations* fop,uint32_t major,char *name)
{
    return sys_fs_create_inode(sb,parent_dentry_inode,fop,major,name);
}

static struct sys_fs_dentry* sys_fs_create_dir(struct sys_fs_superblock *sb,struct sys_fs_inode* parent_dentry_inode,char *name)
{
    struct sys_fs_dentry* dentry =  sys_fs_create_inode(sb,parent_dentry_inode,&sys_fs_file_fops,0,name);
    if(dentry == NULL) return NULL;
    dentry->target_inode->i_mode = S_IFDIR | 0755;
    return dentry;
}
static void dentry_put(struct sys_fs_superblock *sb,struct sys_fs_dentry* dentry)
{
    if(dentry == NULL) return;
    if(dentry->target_inode != NULL)
    {
        spin_lock(&sb->lock);
        if (atomic_dec_and_test(&dentry->target_inode->dentry_count)) //检测inode引用计数是否为0，如果为0则删除
        {
            list_del(&dentry->target_inode->list_node);    
            sys_fs_inode_put(dentry->target_inode);
        }
        spin_unlock(&sb->lock);
    }
    kfree(dentry->name);
    kfree(dentry);
}
static void sys_fs_delete_dentry(struct sys_fs_superblock *sb,struct sys_fs_inode* parent_dentry_inode,struct sys_fs_dentry* delete_dentry){
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
static struct sys_fs_superblock* sys_fs_superblock_create(void) //创建superblock
{
    struct sys_fs_superblock*d_sb = kmalloc(sizeof(struct sys_fs_superblock),GFP_KERNEL);
    if(d_sb == NULL){
        return NULL;        
    }
    d_sb->magic = MAGIC;
    spin_lock_init(&d_sb->lock);
    INIT_LIST_HEAD(&d_sb->inode_list_head);
    struct sys_fs_inode* root_inode = sys_fs_create_empty_inode(d_sb ,&sys_fs_file_fops,0);
    if(root_inode == NULL){
        kfree(d_sb);
        return NULL;
    }
    root_inode->i_mode = S_IFDIR | 0755;
    d_sb->rootinode = root_inode;
    root_inode->i_fop = &sys_fs_file_fops;
    atomic_inc (&root_inode->dentry_count); 
    INIT_LIST_HEAD(&root_inode->dentry_list_head);
    return d_sb;
} 
static void sys_fs_superblock_destory(struct sys_fs_superblock* sb){
    if(sb!= NULL) {
        struct sys_fs_inode * root_inode = sb->rootinode;
        if(!root_inode){
            kfree(root_inode);
        }
        kfree(sb);
    }
}

static struct sys_fs_dentry* sys_fs_lookup(struct sys_fs_inode* dentry_inode,char *name)
{
    if(dentry_inode == NULL) return NULL;
    if ( (dentry_inode->i_mode & S_IFMT) != S_IFDIR ) return NULL;
    struct sys_fs_dentry* pos;
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








static struct super_operations sys_fs_super_operation;
static struct dentry_operations sys_fs_dentry_operation;
static struct inode_operations sys_fs_inode_operation;

static void sys_fs_kill_sb (struct super_block * sb);
static struct inode *sys_fs_alloc_inode(struct super_block *sb);
static void  sys_fs_destroy_inode(struct inode * node); //撤销sys_fs_alloc_inode所做的一切
static int sys_fs_statfs (struct dentry * dentry, struct kstatfs *star);
static struct dentry * sys_fs_get_dentry(struct dentry * vdentry, struct sys_fs_dentry *dentry);
static struct super_block *sys_fs_get_vfs_superblock(struct sys_fs_superblock *sb);
static struct inode *  sys_fs_get_inode(struct sys_fs_inode *,struct super_block*);
static struct dentry *sys_fs_mount(struct file_system_type *fs_type,
    int flags,
    const char *sys__name,
    void *data);

static struct file_system_type fs_type = {
    .name     = "sysfs",
    .fs_flags = 0,
    .init_fs_context = NULL,
    .kill_sb  = sys_fs_kill_sb,
    .mount    = sys_fs_mount,
    .owner    = THIS_MODULE,
};

static struct dentry *sys_fs_mount(struct file_system_type *fs_type,
    int flags,
    const char *sys__name,
    void *data)
{
    struct sys_fs_superblock *fs_superblock= sys_fs_superblock_create();
    if (fs_superblock == NULL){
        return ERR_PTR(-ENOMEM);        
    }
    struct dentry * root = __d_alloc(NULL,"root");
    if(root == NULL){
         sys_fs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);   
    }


    struct sys_fs_dentry* dentry = kmalloc(sizeof(struct sys_fs_dentry),GFP_KERNEL);
    if(dentry == NULL){
       
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    }
    dentry->name = NULL;
    dentry->target_inode = fs_superblock->rootinode;
    dentry->magic =MAGIC;
    root->d_fsdata = dentry;
    struct inode * root_inode = sys_fs_get_inode(dentry->target_inode,NULL);
    if(root_inode == NULL){
        kfree(dentry);
        sys_fs_superblock_destory(fs_superblock);
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    } 
    root->d_inode = root_inode;

    struct super_block *sb =sys_fs_get_vfs_superblock(fs_superblock);
    if(sb == NULL){
        kfree(root);
        kfree(dentry);
        inode_put(root_inode);
        sys_fs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);       
    }
    sb->s_root = root;
    root->d_sb = sb;

    return root;
}



static struct super_operations sys_fs_super_operation = {
    .alloc_inode   = sys_fs_alloc_inode,
    .destroy_inode = sys_fs_destroy_inode, 
    .free_inode    = NULL,         
    .dirty_inode   = NULL,
    .write_inode   = NULL,
    .drop_inode    = NULL,
    .put_super     = sys_fs_kill_sb,
    .sync_fs       = NULL,
    .statfs        = sys_fs_statfs,
};

static struct super_block *sys_fs_get_vfs_superblock(struct sys_fs_superblock *sb) //将文件系统的superblock映射到vfs
{
    if(sb == NULL) return NULL;
    struct super_block * vsb  = alloc_super(NULL);                        //分配了一个superblock(注意释放)
    if(vsb == NULL) return NULL;
    vsb->s_fs_info =   sb;
    vsb->s_d_op    =  &sys_fs_dentry_operation;
    vsb->s_op      =  &sys_fs_super_operation;    
    return vsb;
}


static void sys_fs_kill_sb (struct super_block * sb){                          //释放文件系统实例，所以并不会删除superblock本身只是删除映射 
    put_super(sb);
}

static struct inode *  sys_fs_get_inode(struct sys_fs_inode * sys__node,struct super_block* sb )  //通过该文件系统的inode转化为vfs的标准inode(不修改自身inode，需要分配inode)
{
    if(sys__node == NULL) return NULL;
    struct inode*inode  =  new_inode(sb); //此处动态创建了一个inode,需要后续释放
    if(inode == NULL) return NULL;  
    inode->i_mode    = sys__node->i_mode;
    inode->i_fop     = sys__node->i_fop;
    inode->i_op      = &sys_fs_inode_operation;
    inode->i_private = sys__node;
    return inode;
}

static struct inode *sys_fs_alloc_inode(struct super_block *sb) //创建一个inode，同时创建inode和文件系统的inode。但是没有硬链接
{
    struct sys_fs_superblock *supb = sb->s_fs_info;
    if(supb->magic != MAGIC) return NULL;
     struct sys_fs_inode * inode = sys_fs_create_empty_inode(supb,&sys_fs_file_fops,10); //文件系统分配一个inode
     if(inode == NULL) return NULL;
     struct inode *out_inode = sys_fs_get_inode(inode,sb);
     if(out_inode == NULL){
        kfree(inode);
        return NULL;
     }
     return out_inode;
}



static void  sys_fs_destroy_inode(struct inode * node) //撤销sys_fs_alloc_inode所做的一切
{
    if(node == NULL) return;
    if(node->i_private != NULL) {
        struct sys_fs_inode * sys__node = node->i_private;
        list_del(&sys__node->list_node);
        kfree(sys__node);        
    }
}

static int sys_fs_statfs (struct dentry * dentry, struct kstatfs *star){
    struct sys_fs_dentry * sys_fs_dentry = dentry->d_fsdata;
    struct sys_fs_inode * sys_fs_node = sys_fs_dentry->target_inode;
    star->f_type = MAGIC;
    star->f_bsize  = 0;
    star->f_blocks = 1;
    star->f_bfree  = 0;
    star->f_bavail = 0;
    star->f_files  = 128;
    star->f_ffree  = 127;
    return 0;
}



static int sys_fs_create(struct mnt_idmap * map, struct inode * dir,struct dentry * dentry,umode_t mode, bool bool);
static struct dentry *sys_fs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags);
static int sys_fs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry);
static int sys_fs_unlink(struct inode *dir, struct dentry *dentry);
static int sys_fs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode);
static int sys_fs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr);
static int  sys_fs_getattr (struct mnt_idmap *dmp, const struct path * path,struct kstat *stat, u32 u, unsigned int i);
static int sys_fs_rmdir(struct inode *	inode  ,struct dentry *dentry);

static struct inode_operations sys_fs_inode_operation = {
    .create = sys_fs_create,
    .lookup = sys_fs_vfs_lookup,
    .link   = sys_fs_link,
    .unlink = sys_fs_unlink,
    .mkdir  = sys_fs_mkdir,
    .rmdir  = sys_fs_rmdir,
    .setattr = sys_fs_setattr,
    .getattr = sys_fs_getattr
};

static struct dentry * sys_fs_get_dentry(struct dentry * vdentry, struct sys_fs_dentry *dentry) //通过dentry获得标准的dentry。但是此处的dentry不是自己动态分配的
{
    if(dentry == NULL || vdentry == NULL) return NULL;
    vdentry->d_fsdata = dentry;
    return vdentry;
}

static int sys_fs_create(struct mnt_idmap * map, struct inode * dir,struct dentry * dentry,umode_t mode, bool bool)
{
    char *file_name = d_getname(dentry);

    struct sys_fs_inode *dinode = dir->i_private;
    struct sys_fs_dentry* file = sys_fs_lookup(dinode,file_name);
    if(file == NULL){
        file =  sys_fs_create_file(dinode->sb,dinode,&sys_fs_file_fops,10,file_name);
    }
    if (file == NULL){                               //确定文件是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    if( sys_fs_get_dentry(dentry,file) == NULL){     //构建dentry    
     return -1;
    }  

    struct inode * new_file_inode = sys_fs_get_inode(file->target_inode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); //通知文件系统加入缓存，并进行资源绑定
   return 0;
} 

static struct dentry *sys_fs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags){
    
    struct sys_fs_inode *dir_inode = (struct sys_fs_inode *)dir->i_private; //获取文件系统本身的inode
    char *name = dentry->d_name.name;                           
    struct sys_fs_dentry *pos;    
    pos = sys_fs_lookup(dir_inode,name);       //查询获得dentry
    if(pos == NULL) return NULL;

    if( sys_fs_get_dentry(dentry,pos) == NULL)
        return NULL;

    struct inode * find_inode = sys_fs_get_inode(pos->target_inode,dir->i_sb);
    if(find_inode == NULL)
        return NULL;
    
    return d_add(dentry,find_inode);
    return NULL;
}


static int sys_fs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry)
{
    struct sys_fs_inode *old_inode = (struct sys_fs_inode *)old_dentry->d_inode->i_private;
    struct sys_fs_inode *dir_inode = (struct sys_fs_inode *)dir->i_private;
    struct sys_fs_dentry *new_dentry = sys_fs_create_dentry(dir_inode, old_inode, dentry->d_name.name);
    if (new_dentry == NULL) {
        return -ENOMEM;
    }
    sys_fs_get_dentry(dentry,new_dentry);
    dentry->d_sb = dir->i_sb;
    d_add(dentry,dentry->d_inode);
    return 0;
}

static int sys_fs_unlink(struct inode *dir, struct dentry *dentry)
{
    struct sys_fs_inode *parent_inode = (struct sys_fs_inode *)dir->i_private;
    struct sys_fs_dentry *delete_dentry = (struct sys_fs_dentry *)dentry->d_fsdata;
    sys_fs_delete_dentry(parent_inode->sb, parent_inode, delete_dentry);
    return 0;
}


static int sys_fs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    char *file_name = d_getname(dentry);

    struct sys_fs_inode *dinode = dir->i_private;
    struct sys_fs_dentry* file = sys_fs_lookup(dinode,file_name);
    if(file == NULL){
        file =  sys_fs_create_dir(dinode->sb,dinode,file_name);
    }
    if (file == NULL){                               //确定文件是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    
    if( sys_fs_get_dentry(dentry,file) == NULL){     //构建dentry    
     return -1;
    }  
    struct inode * new_file_inode = sys_fs_get_inode(dinode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); //通知文件系统加入缓存，并进行资源绑定
   return 0;
}


static int sys_fs_release_dentry(struct dentry *dentry){
    struct sys_fs_dentry *fs_dentry = dentry->d_fsdata;
    dentry_put(fs_dentry->target_inode->sb, fs_dentry);
}
static int sys_fs_rmdir(struct inode *	dir  ,struct dentry *dentry){
    sys_fs_release_dentry(dentry);
    return simple_unlink(dir,dentry);
}

static int sys_fs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr)
{
    struct sys_fs_dentry* file = dentry->d_fsdata;
    file->target_inode->i_mode = (iattr->ia_mode & S_IFMT) | iattr->ia_mode;
    return 0;
}
static int  sys_fs_getattr (struct mnt_idmap *dmp, const struct path * path,struct kstat *stat, u32 u, unsigned int i)
{
    struct dentry *d = path->dentry;
    struct sys_fs_dentry* file = d->d_fsdata;
    struct sys_fs_inode *inode;

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

static struct dentry_operations sys_fs_dentry_operation = {
    .d_release = sys_fs_release_dentry,
};

static int __init sys_fs_ops_init(void){
    register_filesystem(&fs_type);
    return 0;
}

fs_initcall(sys_fs_ops_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("你");
MODULE_DESCRIPTION("简易设备文件系统 sys_fs 测试");







