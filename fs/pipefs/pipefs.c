#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sched.h>




#define MAGIC 12345678
struct pipefs_dentry {
    int                    magic;
    char *name;                             // 名字
    struct pipefs_inode *target_inode;       // 该dentry指向的目标inode
    struct list_head list_node;             // 挂载到父目录inode中的listhead中
};
struct pipefs_superblock {
    int                    magic;
    spinlock_t             lock;
    struct pipefs_inode *rootinode;
    struct list_head inode_list_head;       // 该superblock下的inode链表头
};


int pipe_buffer_read(struct pipe_buffer *buf, char* data) 
{
    spin_lock(&buf->lock);

    if (buf->head == buf->tail) {
        spin_unlock(&buf->lock);
        return -1; // Buffer empty
    }

    data[0] = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % buf->size;

    spin_unlock(&buf->lock);
    return 0;
}

int pipe_buffer_write(struct pipe_buffer *buf, char data)
{
    spin_lock(&buf->lock);

    if ((buf->head + 1) % buf->size == buf->tail) {
        spin_unlock(&buf->lock);
        return -1; // Buffer full
    }

    buf->buffer[buf->head] = data;
    buf->head = (buf->head + 1) % buf->size;

    spin_unlock(&buf->lock);
    return 0;
}


static int pipefs_open(struct inode *inode, struct file *file)
{
    struct pipefs_inode *p_inode = inode->i_private;
    if(p_inode->buf == NULL)
    {
        p_inode->buf = kmalloc(sizeof(struct pipe_buffer),GFP_KERNEL);
        if(!p_inode->buf)
           return -1;
    }
    return 0;
}

static int pipefs_release(struct inode *inode, struct file *file){
    pr_info("pipefs test close\n");
    return 0;
}

static int pipefs_read(struct file *file, char __user * data, size_t size, loff_t *offset)
{    
    for(int i = 0;i < size;i++)
    {
        while (pipe_buffer_read(((struct pipefs_inode*)(file->f_inode->i_private))->buf,&data[i]) < 0){
            __delay(1);
        }
    }
    return 0;
}

static int pipefs_write(struct file *file, char __user * data, size_t size, loff_t *offset){
    int i;
    for(i = 0;i < size;i++)
    {
        while (pipe_buffer_write(((struct pipefs_inode*)(file->f_inode->i_private))->buf,data[i]) < 0){
            __delay(1);
        }
    }
    return 0;
}





static struct file_operations pipefs_file_fops = {
    .owner = THIS_MODULE,
    .open = pipefs_open,
    .release = pipefs_release,
    .read  = pipefs_read,
    .write = pipefs_write
};

static struct pipefs_inode *pipefs_create_empty_inode(struct pipefs_superblock *sb,struct file_operations *fop, uint32_t major) //普通地创建一个inode
{
    if(fop == NULL) return NULL;
    struct pipefs_inode *pipe_inode = kmalloc(sizeof(struct pipefs_inode),GFP_KERNEL);
    if(pipe_inode == NULL) return NULL;
    pipe_inode->magic = MAGIC;
    pipe_inode->major = major;
    pipe_inode->i_fop = fop;
    pipe_inode->sb    = sb;
    pipe_inode->i_mode = S_IFREG | 0755;
    atomic_set(&pipe_inode->dentry_count,0);
    spin_lock_init(&pipe_inode->lock);
    INIT_LIST_HEAD(&pipe_inode->list_node);
    INIT_LIST_HEAD(&pipe_inode->dentry_list_head);
    
    pipe_inode->buf = NULL;

    spin_lock(&sb->lock);
    list_add(&pipe_inode->list_node,&sb->inode_list_head);
    spin_unlock(&sb->lock);
    
    return pipe_inode;
}
static void pipefs_inode_put(struct pipefs_inode *node){
    if(node != NULL)kfree(node);
}
static struct pipefs_dentry* pipefs_create_dentry(struct pipefs_inode* parent_dentry_inode,struct pipefs_inode* target_inode,char *name) //创建硬连接
{
    if(parent_dentry_inode == NULL || name == NULL || target_inode == NULL) return NULL;

    struct pipefs_dentry* new_dentry =  kmalloc(sizeof(struct pipefs_dentry),GFP_KERNEL);
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
static struct pipefs_dentry* pipefs_create_inode( struct pipefs_superblock *sb,struct pipefs_inode* parent_dentry_inode,struct file_operations* fop,uint32_t major,char *name)
{
    struct pipefs_inode * new_inode = pipefs_create_empty_inode(sb,fop,major);
    if(new_inode == NULL){
        return NULL;
    }   
    struct pipefs_dentry* new_dentry = pipefs_create_dentry(parent_dentry_inode,new_inode,name); //创建硬连接
    if(new_dentry == NULL){
        spin_lock(&sb->lock);
        list_del(&new_inode->list_node);
        spin_unlock(&sb->lock);
        pipefs_inode_put(new_inode);
        return NULL;
    }
    return new_dentry;
}
static struct pipefs_dentry* pipefs_create_file(struct pipefs_superblock *sb,struct pipefs_inode* parent_dentry_inode,struct file_operations* fop,uint32_t major,char *name)
{
    return pipefs_create_inode(sb,parent_dentry_inode,fop,major,name);
}

static struct pipefs_dentry* pipefs_create_dir(struct pipefs_superblock *sb,struct pipefs_inode* parent_dentry_inode,char *name)
{
    struct pipefs_dentry* dentry =  pipefs_create_inode(sb,parent_dentry_inode,&pipefs_file_fops,0,name);
    if(dentry == NULL) return NULL;
    dentry->target_inode->i_mode = S_IFDIR | 0755;
    return dentry;
}
static void dentry_put(struct pipefs_superblock *sb,struct pipefs_dentry* dentry)
{
    if(dentry == NULL) return;
    if(dentry->target_inode != NULL)
    {
        spin_lock(&sb->lock);
        if (atomic_dec_and_test(&dentry->target_inode->dentry_count)) //检测inode引用计数是否为0，如果为0则删除
        {
            list_del(&dentry->target_inode->list_node);    
            pipefs_inode_put(dentry->target_inode);
        }
        spin_unlock(&sb->lock);
    }
    kfree(dentry->name);
    kfree(dentry);
}
static void pipefs_delete_dentry(struct pipefs_superblock *sb,struct pipefs_inode* parent_dentry_inode,struct pipefs_dentry* delete_dentry){
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
static struct pipefs_superblock* pipefs_superblock_create(void) //创建superblock
{
    struct pipefs_superblock*d_sb = kmalloc(sizeof(struct pipefs_superblock),GFP_KERNEL);
    if(d_sb == NULL){
        return NULL;        
    }
    d_sb->magic = MAGIC;
    spin_lock_init(&d_sb->lock);
    INIT_LIST_HEAD(&d_sb->inode_list_head);
    struct pipefs_inode* root_inode = pipefs_create_empty_inode(d_sb ,&pipefs_file_fops,0);
    if(root_inode == NULL){
        kfree(d_sb);
        return NULL;
    }
    root_inode->i_mode = S_IFDIR | 0755;
    d_sb->rootinode = root_inode;
    root_inode->i_fop = &pipefs_file_fops;
    atomic_inc (&root_inode->dentry_count); 
    INIT_LIST_HEAD(&root_inode->dentry_list_head);
    return d_sb;
} 
static void pipefs_superblock_destory(struct pipefs_superblock* sb){
    if(sb!= NULL) {
        struct pipefs_inode * root_inode = sb->rootinode;
        if(!root_inode){
            kfree(root_inode);
        }
        kfree(sb);
    }
}

static struct pipefs_dentry* pipefs_lookup(struct pipefs_inode* dentry_inode,char *name)
{
    if(dentry_inode == NULL) return NULL;
    if ( (dentry_inode->i_mode & S_IFMT) != S_IFDIR ) return NULL;
    struct pipefs_dentry* pos;
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









static struct super_operations pipefs_super_operation;
static struct dentry_operations pipefs_dentry_operation;
static struct inode_operations pipefs_inode_operation;

static void pipefs_kill_sb (struct super_block * sb);
static struct inode *pipefs_alloc_inode(struct super_block *sb);
static void  pipefs_destroy_inode(struct inode * node); //撤销pipefs_alloc_inode所做的一切
static int pipefs_statfs (struct dentry * dentry, struct kstatfs *star);
static struct dentry * pipefs_get_dentry(struct dentry * vdentry, struct pipefs_dentry *dentry);
static struct super_block *pipefs_get_vfs_superblock(struct pipefs_superblock *sb);
static struct inode *  pipefs_get_inode(struct pipefs_inode *,struct super_block*);
static struct dentry *pipefs_mount(struct file_system_type *fs_type,
    int flags,
    const char *pipe_name,
    void *data);

static struct file_system_type fs_type = {
    .name     = "pipefs",
    .fs_flags = 0,
    .init_fs_context = NULL,
    .kill_sb  = pipefs_kill_sb,
    .mount    = pipefs_mount,
    .owner    = THIS_MODULE,
};

static struct dentry *pipefs_mount(struct file_system_type *fs_type,
    int flags,
    const char *pipe_name,
    void *data)
{
    struct pipefs_superblock *fs_superblock= pipefs_superblock_create();
    if (fs_superblock == NULL){
        return ERR_PTR(-ENOMEM);        
    }
    struct dentry * root = __d_alloc(NULL,"root");
    if(root == NULL){
        pipefs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);   
    }


    struct pipefs_dentry* dentry = kmalloc(sizeof(struct pipefs_dentry),GFP_KERNEL);
    if(dentry == NULL){
       
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    }
    dentry->name = NULL;
    dentry->target_inode = fs_superblock->rootinode;
    dentry->magic =MAGIC;
    root->d_fsdata = dentry;
    struct inode * root_inode = pipefs_get_inode(dentry->target_inode,NULL);
    if(root_inode == NULL){
        kfree(dentry);
        pipefs_superblock_destory(fs_superblock);
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    } 
    root->d_inode = root_inode;

    struct super_block *sb =pipefs_get_vfs_superblock(fs_superblock);
    if(sb == NULL){
        kfree(root);
        kfree(dentry);
        inode_put(root_inode);
        pipefs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);       
    }
    sb->s_root = root;
    root->d_sb = sb;

    return root;
}



static struct super_operations pipefs_super_operation = {
    .alloc_inode   = pipefs_alloc_inode,
    .destroy_inode = pipefs_destroy_inode, 
    .free_inode    = NULL,         
    .dirty_inode   = NULL,
    .write_inode   = NULL,
    .drop_inode    = NULL,
    .put_super     = pipefs_kill_sb,
    .sync_fs       = NULL,
    .statfs        = pipefs_statfs,
};

static struct super_block *pipefs_get_vfs_superblock(struct pipefs_superblock *sb) //将文件系统的superblock映射到vfs
{
    if(sb == NULL) return NULL;
    struct super_block * vsb  = alloc_super(NULL);                        //分配了一个superblock(注意释放)
    if(vsb == NULL) return NULL;
    vsb->s_fs_info =   sb;
    vsb->s_d_op    =  &pipefs_dentry_operation;
    vsb->s_op      =  &pipefs_super_operation;    
    return vsb;
}


static void pipefs_kill_sb (struct super_block * sb){                          //释放文件系统实例，所以并不会删除superblock本身只是删除映射 
    put_super(sb);
}

static struct inode *  pipefs_get_inode(struct pipefs_inode * pipe_node,struct super_block* sb )  //通过该文件系统的inode转化为vfs的标准inode(不修改自身inode，需要分配inode)
{
    if(pipe_node == NULL) return NULL;
    struct inode*inode  =  new_inode(sb); //此处动态创建了一个inode,需要后续释放
    if(inode == NULL) return NULL;  
    inode->i_mode    = pipe_node->i_mode;
    inode->i_fop     = pipe_node->i_fop;
    inode->i_op      = &pipefs_inode_operation;
    inode->i_private = pipe_node;
    return inode;
}

static struct inode *pipefs_alloc_inode(struct super_block *sb) //创建一个inode，同时创建inode和文件系统的inode。但是没有硬链接
{
    struct pipefs_superblock *supb = sb->s_fs_info;
    if(supb->magic != MAGIC) return NULL;
     struct pipefs_inode * inode = pipefs_create_empty_inode(supb,&pipefs_file_fops,10); //文件系统分配一个inode
     if(inode == NULL) return NULL;
     struct inode *out_inode = pipefs_get_inode(inode,sb);
     if(out_inode == NULL){
        kfree(inode);
        return NULL;
     }
     return out_inode;
}



static void  pipefs_destroy_inode(struct inode * node) //撤销pipefs_alloc_inode所做的一切
{
    if(node == NULL) return;
    if(node->i_private != NULL) {
        struct pipefs_inode * pipe_node = node->i_private;
        list_del(&pipe_node->list_node);
        kfree(pipe_node);        
    }
}

static int pipefs_statfs (struct dentry * dentry, struct kstatfs *star){
    struct pipefs_dentry * pipefs_dentry = dentry->d_fsdata;
    struct pipefs_inode * pipefs_node = pipefs_dentry->target_inode;
    star->f_type = MAGIC;
    star->f_bsize  = 0;
    star->f_blocks = 1;
    star->f_bfree  = 0;
    star->f_bavail = 0;
    star->f_files  = 128;
    star->f_ffree  = 127;
    return 0;
}



static int pipefs_create(struct mnt_idmap * map, struct inode * dir,struct dentry * dentry,umode_t mode, bool bool);
static struct dentry *pipefs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags);
static int pipefs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry);
static int pipefs_unlink(struct inode *dir, struct dentry *dentry);
static int pipefs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode);
static int pipefs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr);
static int  pipefs_getattr (struct mnt_idmap *dmp, const struct path * path,struct kstat *stat, u32 u, unsigned int i);
static int pipefs_rmdir(struct inode *	inode  ,struct dentry *dentry);

static struct inode_operations pipefs_inode_operation = {
    .create = pipefs_create,
    .lookup = pipefs_vfs_lookup,
    .link   = pipefs_link,
    .unlink = pipefs_unlink,
    .mkdir  = pipefs_mkdir,
    .rmdir  = pipefs_rmdir,
    .setattr = pipefs_setattr,
    .getattr = pipefs_getattr
};

static struct dentry * pipefs_get_dentry(struct dentry * vdentry, struct pipefs_dentry *dentry) //通过dentry获得标准的dentry。但是此处的dentry不是自己动态分配的
{
    if(dentry == NULL || vdentry == NULL) return NULL;
    vdentry->d_fsdata = dentry;
    return vdentry;
}

static int pipefs_create(struct mnt_idmap * map, struct inode * dir,struct dentry * dentry,umode_t mode, bool bool)
{
    char *file_name = d_getname(dentry);

    struct pipefs_inode *dinode = dir->i_private;
    struct pipefs_dentry* file = pipefs_lookup(dinode,file_name);
    if(file == NULL){
        file =  pipefs_create_file(dinode->sb,dinode,&pipefs_file_fops,10,file_name);
    }
    if (file == NULL){                               //确定文件是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    if( pipefs_get_dentry(dentry,file) == NULL){     //构建dentry    
     return -1;
    }  

    struct inode * new_file_inode = pipefs_get_inode(dinode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); //通知文件系统加入缓存，并进行资源绑定
   return 0;
} 

static struct dentry *pipefs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags){
    
    struct pipefs_inode *dir_inode = (struct pipefs_inode *)dir->i_private; //获取文件系统本身的inode
    char *name = dentry->d_name.name;                           
    struct pipefs_dentry *pos;    
    pos = pipefs_lookup(dir_inode,name);       //查询获得dentry
    if(pos == NULL) return NULL;

    if( pipefs_get_dentry(dentry,pos) == NULL)
        return NULL;

    struct inode * find_inode = pipefs_get_inode(pos->target_inode,dir->i_sb);
    if(find_inode == NULL)
        return NULL;
    
    return d_add(dentry,find_inode);
    return NULL;
}


static int pipefs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry)
{
    struct pipefs_inode *old_inode = (struct pipefs_inode *)old_dentry->d_inode->i_private;
    struct pipefs_inode *dir_inode = (struct pipefs_inode *)dir->i_private;
    struct pipefs_dentry *new_dentry = pipefs_create_dentry(dir_inode, old_inode, dentry->d_name.name);
    if (new_dentry == NULL) {
        return -ENOMEM;
    }
    pipefs_get_dentry(dentry,new_dentry);
    dentry->d_sb = dir->i_sb;
    d_add(dentry,dentry->d_inode);
    return 0;
}

static int pipefs_unlink(struct inode *dir, struct dentry *dentry)
{
    struct pipefs_inode *parent_inode = (struct pipefs_inode *)dir->i_private;
    struct pipefs_dentry *delete_dentry = (struct pipefs_dentry *)dentry->d_fsdata;
    pipefs_delete_dentry(parent_inode->sb, parent_inode, delete_dentry);
    return 0;
}


static int pipefs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    char *file_name = d_getname(dentry);

    struct pipefs_inode *dinode = dir->i_private;
    struct pipefs_dentry* file = pipefs_lookup(dinode,file_name);
    if(file == NULL){
        file =  pipefs_create_dir(dinode->sb,dinode,file_name);
    }
    if (file == NULL){                               //确定文件是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    
    if( pipefs_get_dentry(dentry,file) == NULL){     //构建dentry    
     return -1;
    }  
    struct inode * new_file_inode = pipefs_get_inode(dinode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); //通知文件系统加入缓存，并进行资源绑定
   return 0;
}


static int pipefs_release_dentry(struct dentry *dentry){
    struct pipefs_dentry *fs_dentry = dentry->d_fsdata;
    dentry_put(fs_dentry->target_inode->sb, fs_dentry);
}
static int pipefs_rmdir(struct inode *	dir  ,struct dentry *dentry){
    pipefs_release_dentry(dentry);
    return simple_unlink(dir,dentry);
}

static int pipefs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr)
{
    struct pipefs_dentry* file = dentry->d_fsdata;
    file->target_inode->i_mode = (iattr->ia_mode & S_IFMT) | iattr->ia_mode;
    return 0;
}
static int  pipefs_getattr (struct mnt_idmap *dmp, const struct path * path,struct kstat *stat, u32 u, unsigned int i)
{
    struct dentry *d = path->dentry;
    struct pipefs_dentry* file = d->d_fsdata;
    struct pipefs_inode *inode;

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

static struct dentry_operations pipefs_dentry_operation = {
    .d_release = pipefs_release_dentry,
};

static int __init pipefs_ops_init(void){
    register_filesystem(&fs_type);
    return 0;
}

fs_initcall(pipefs_ops_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("你");
MODULE_DESCRIPTION("简易设备文件系统 pipefs 测试");







