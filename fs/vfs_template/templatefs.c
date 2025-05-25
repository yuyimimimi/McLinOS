#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>

/*
 * File System Magic Number - Used for validation
 * 文件系统魔数 - 用于验证
 */
#define MAGIC 12345678

/*
 * Inode structure for template file system
 * 模板文件系统的inode结构
 */
struct template_fs_inode {
    int                     magic;              // Magic number for validation 验证魔数
    uint32_t                i_mode;             // File type and permissions 文件类型和权限
    uint32_t                major;              // Major device number 主设备号
    atomic_t                dentry_count;       // Reference count 引用计数
    struct file_operations *i_fop;              // File operations 文件操作
    struct list_head        list_node;          // Node for superblock's inode list 挂在superblock的inode链表
    struct list_head        dentry_list_head;   // List head for dentries under this inode (if directory) 如果是目录项，这里存储inode下的所有dentry链表头
    spinlock_t              lock;               // Spinlock for synchronization 自旋锁
    
    struct block_template_ice *btemplate_;      // Block template pointer 块模板指针

    struct template_fs_superblock *sb;          // Pointer to superblock 指向superblock的指针
};

/*
 * Directory entry (dentry) structure for template file system
 * 模板文件系统的目录项(dentry)结构
 */
struct template_fs_dentry {
    int                    magic;               // Magic number for validation 验证魔数
    char *name;                                 // Name of the entry 名字
    struct template_fs_inode *target_inode;     // Target inode this dentry points to 该dentry指向的目标inode
    struct list_head list_node;                 // Node for parent directory's dentry list 挂载到父目录inode中的listhead中
};

/*
 * Superblock structure for template file system
 * 模板文件系统的superblock结构
 */
struct template_fs_superblock {
    int                    magic;               // Magic number for validation 验证魔数
    spinlock_t             lock;                // Spinlock for synchronization 自旋锁
    struct template_fs_inode *rootinode;        // Root inode 根inode
    struct list_head inode_list_head;           // List head for all inodes under this superblock 该superblock下的inode链表头
};

/* File operation functions 文件操作函数 */

/*
 * Open file operation
 * 打开文件操作
 */
static int template_fs_open(struct inode *inode, struct file *file){
    early_printk("template_fs test open\n");
    return 0;
}

/*
 * Release (close) file operation
 * 释放(关闭)文件操作
 */
static int template_fs_release(struct inode *inode, struct file *file){
    early_printk("template_fs test close\n");
    return 0;
}

/*
 * Read file operation
 * 读文件操作
 */
static int template_fs_read(struct file *file, char __user * data, size_t size, loff_t *offset){
    early_printk("template_fs test read\n");
    return 0;
}

/*
 * Write file operation
 * 写文件操作
 */
static int template_fs_write(struct file *file, char __user * data, size_t size, loff_t *offset){
    early_printk("template_fs test write\n");
    return 0;
}

/* File operations structure 文件操作结构体 */
static struct file_operations template_fs_file_fops = {
    .owner   = THIS_MODULE,
    .open    = template_fs_open,
    .release = template_fs_release,
    .read    = template_fs_read,
    .write   = template_fs_write
};

/*
 * Create an empty inode
 * 创建一个空的inode
 * @param sb: Superblock pointer superblock指针
 * @param fop: File operations pointer 文件操作指针
 * @param major: Major device number 主设备号
 * @return: Pointer to created inode, or NULL on failure 成功返回inode指针，失败返回NULL
 */
static struct template_fs_inode *template_fs_create_empty_inode(struct template_fs_superblock *sb,
                                                              struct file_operations *fop, 
                                                              uint32_t major)
{
    if(fop == NULL) return NULL;
    struct template_fs_inode *template__inode = kmalloc(sizeof(struct template_fs_inode),GFP_KERNEL);
    if(template__inode == NULL) return NULL;
    
    // Initialize inode fields 初始化inode字段
    template__inode->magic = MAGIC;
    template__inode->major = major;
    template__inode->i_fop = fop;
    template__inode->sb    = sb;
    template__inode->i_mode = S_IFREG | 0755;  // Default to regular file 默认为普通文件
    atomic_set(&template__inode->dentry_count,0);
    spin_lock_init(&template__inode->lock);
    INIT_LIST_HEAD(&template__inode->list_node);
    INIT_LIST_HEAD(&template__inode->dentry_list_head);
    
    // Add to superblock's inode list 添加到superblock的inode列表
    spin_lock(&sb->lock);
    list_add(&template__inode->list_node,&sb->inode_list_head);
    spin_unlock(&sb->lock);
    
    return template__inode;
}

/*
 * Free an inode
 * 释放一个inode
 * @param node: Inode to free 要释放的inode
 */
static void template_fs_inode_put(struct template_fs_inode *node){
    if(node != NULL) kfree(node);
}

/*
 * Create a dentry (hard link)
 * 创建一个dentry(硬链接)
 * @param parent_dentry_inode: Parent directory's inode 父目录的inode
 * @param target_inode: Target inode this dentry points to 该dentry指向的目标inode
 * @param name: Name of the dentry dentry名称
 * @return: Pointer to created dentry, or NULL on failure 成功返回dentry指针，失败返回NULL
 */
static struct template_fs_dentry* template_fs_create_dentry(struct template_fs_inode* parent_dentry_inode,
                                                          struct template_fs_inode* target_inode,
                                                          char *name)
{
    if(parent_dentry_inode == NULL || name == NULL || target_inode == NULL) 
        return NULL;

    struct template_fs_dentry* new_dentry = kmalloc(sizeof(struct template_fs_dentry),GFP_KERNEL);
    if(new_dentry == NULL) return NULL;
    
    // Initialize dentry fields 初始化dentry字段
    new_dentry->magic = MAGIC;
    new_dentry->name = NULL;
    new_dentry->name = kmalloc(strlen(name) + 1,GFP_KERNEL);
    if(new_dentry->name == NULL) {
        kfree(new_dentry);
        return NULL;
    }
    strcpy(new_dentry->name,name);
    new_dentry->target_inode = target_inode;
    INIT_LIST_HEAD(&new_dentry->list_node);
    
    // Increase reference count of target inode 增加目标inode的引用计数
    atomic_inc(&target_inode->dentry_count);
    
    // Add to parent directory's dentry list 添加到父目录的dentry列表
    spin_lock(&parent_dentry_inode->lock);
    list_add(&new_dentry->list_node,&parent_dentry_inode->dentry_list_head);
    spin_unlock(&parent_dentry_inode->lock);
    
    return new_dentry;
}

/*
 * Create an inode with associated dentry
 * 创建一个带有关联dentry的inode
 * @param sb: Superblock pointer superblock指针
 * @param parent_dentry_inode: Parent directory's inode 父目录的inode
 * @param fop: File operations pointer 文件操作指针
 * @param major: Major device number 主设备号
 * @param name: Name of the entry 条目名称
 * @return: Pointer to created dentry, or NULL on failure 成功返回dentry指针，失败返回NULL 
 */
static struct template_fs_dentry* template_fs_create_inode(struct template_fs_superblock *sb,
                                                         struct template_fs_inode* parent_dentry_inode,
                                                         struct file_operations* fop,
                                                         uint32_t major,
                                                         char *name)
{
    struct template_fs_inode * new_inode = template_fs_create_empty_inode(sb,fop,major);
    if(new_inode == NULL){
        return NULL;
    }   
    
    struct template_fs_dentry* new_dentry = template_fs_create_dentry(parent_dentry_inode,new_inode,name);
    if(new_dentry == NULL){
        // Cleanup if failed 如果失败则清理
        spin_lock(&sb->lock);
        list_del(&new_inode->list_node);
        spin_unlock(&sb->lock);
        template_fs_inode_put(new_inode);
        return NULL;
    }
    return new_dentry;
}

/*
 * Create a file (wrapper around template_fs_create_inode)
 * 创建一个文件(template_fs_create_inode的包装)
 */
static struct template_fs_dentry* template_fs_create_file(struct template_fs_superblock *sb,
                                                        struct template_fs_inode* parent_dentry_inode,
                                                        struct file_operations* fop,
                                                        uint32_t major,
                                                        char *name)
{
    return template_fs_create_inode(sb,parent_dentry_inode,fop,major,name);
}

/*
 * Create a directory
 * 创建一个目录
 * @param sb: Superblock pointer superblock指针
 * @param parent_dentry_inode: Parent directory's inode 父目录的inode
 * @param name: Name of the directory 目录名称
 * @return: Pointer to created dentry, or NULL on failure 成功返回dentry指针，失败返回NULL
 */
static struct template_fs_dentry* template_fs_create_dir(struct template_fs_superblock *sb,
                                                       struct template_fs_inode* parent_dentry_inode,
                                                       char *name)
{
    struct template_fs_dentry* dentry = template_fs_create_inode(sb,parent_dentry_inode,
                                                               &template_fs_file_fops,0,name);
    if(dentry == NULL) return NULL;
    dentry->target_inode->i_mode = S_IFDIR | 0755;  // Set as directory 设置为目录
    return dentry;
}

/*
 * Free a dentry
 * 释放一个dentry
 * @param sb: Superblock pointer superblock指针
 * @param dentry: Dentry to free 要释放的dentry
 */
static void dentry_put(struct template_fs_superblock *sb, struct template_fs_dentry* dentry)
{
    if(dentry == NULL) return;
    
    if(dentry->target_inode != NULL) {
        spin_lock(&sb->lock);
        // Check if reference count reaches zero 检查引用计数是否为0
        if (atomic_dec_and_test(&dentry->target_inode->dentry_count)) {
            list_del(&dentry->target_inode->list_node);    
            template_fs_inode_put(dentry->target_inode);
        }
        spin_unlock(&sb->lock);
    }
    kfree(dentry->name);
    kfree(dentry);
}

/*
 * Delete a dentry
 * 删除一个dentry
 * @param sb: Superblock pointer superblock指针
 * @param parent_dentry_inode: Parent directory's inode 父目录的inode
 * @param delete_dentry: Dentry to delete 要删除的dentry
 */
static void template_fs_delete_dentry(struct template_fs_superblock *sb,
                                    struct template_fs_inode* parent_dentry_inode,
                                    struct template_fs_dentry* delete_dentry)
{
    if(parent_dentry_inode != NULL) 
        spin_lock(&parent_dentry_inode->lock);
    
    // If directory, check if it's empty 如果是目录，检查是否为空
    if ((delete_dentry->target_inode->i_mode & S_IFMT) == S_IFDIR ) {
        if(!list_empty(&delete_dentry->target_inode->dentry_list_head)) {
            if(parent_dentry_inode != NULL) 
                spin_unlock(&parent_dentry_inode->lock);
            return;  // Directory not empty 目录不为空
        }
    }
    
    // Remove from parent directory 从父目录中移除
    list_del(&delete_dentry->list_node);
    if(parent_dentry_inode != NULL) 
        spin_unlock(&parent_dentry_inode->lock);
    
    dentry_put(sb,delete_dentry);
}

/*
 * Create a superblock
 * 创建一个superblock
 * @return: Pointer to created superblock, or NULL on failure 成功返回superblock指针，失败返回NULL
 */
static struct template_fs_superblock* template_fs_superblock_create(void)
{
    struct template_fs_superblock*d_sb = kmalloc(sizeof(struct template_fs_superblock),GFP_KERNEL);
    if(d_sb == NULL){
        return NULL;        
    }
    
    // Initialize superblock fields 初始化superblock字段
    d_sb->magic = MAGIC;
    spin_lock_init(&d_sb->lock);
    INIT_LIST_HEAD(&d_sb->inode_list_head);
    
    // Create root inode 创建根inode
    struct template_fs_inode* root_inode = template_fs_create_empty_inode(d_sb,&template_fs_file_fops,0);
    if(root_inode == NULL){
        kfree(d_sb);
        return NULL;
    }
    
    // Set root inode properties 设置根inode属性
    root_inode->i_mode = S_IFDIR | 0755;  // Directory 目录
    d_sb->rootinode = root_inode;
    root_inode->i_fop = &template_fs_file_fops;
    atomic_inc(&root_inode->dentry_count); 
    INIT_LIST_HEAD(&root_inode->dentry_list_head);
    
    return d_sb;
}

/*
 * Destroy a superblock
 * 销毁一个superblock
 * @param sb: Superblock to destroy 要销毁的superblock
 */
static void template_fs_superblock_destory(struct template_fs_superblock* sb){
    if(sb!= NULL) {
        struct template_fs_inode * root_inode = sb->rootinode;
        if(!root_inode){
            kfree(root_inode);
        }
        kfree(sb);
    }
}

/*
 * Lookup a dentry by name in a directory
 * 在目录中按名称查找dentry
 * @param dentry_inode: Directory inode 目录inode
 * @param name: Name to lookup 要查找的名称
 * @return: Pointer to found dentry, or NULL if not found 找到返回dentry指针，未找到返回NULL
 */
static struct template_fs_dentry* template_fs_lookup(struct template_fs_inode* dentry_inode, char *name)
{
    if(dentry_inode == NULL) return NULL;
    if ((dentry_inode->i_mode & S_IFMT) != S_IFDIR) return NULL;  // Not a directory 不是目录
    
    struct template_fs_dentry* pos;
    spin_lock(&dentry_inode->lock);
    list_for_each_entry(pos, &dentry_inode->dentry_list_head, list_node) {
        if (strcmp(pos->name, name) == 0) {
            spin_unlock(&dentry_inode->lock);
            return pos;
        }
    }
    spin_unlock(&dentry_inode->lock);
    return NULL;
}



static int template_fs_iterate(struct file *file, struct dir_context *ctx)
{
    struct template_fs_inode *inode = file->private_data;
    struct template_fs_dentry *pos;
    loff_t curr = 0;

    if ((inode->i_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;

    spin_lock(&inode->lock);
    list_for_each_entry(pos, &inode->dentry_list_head, list_node) {
        if (curr++ < ctx->pos)
            continue;

        if (!dir_emit(ctx, pos->name, strlen(pos->name),
                      curr, 0,  // inode number 可为 0 或 pos->target_inode->major
                      (pos->target_inode->i_mode & S_IFDIR) ? DT_DIR : DT_REG)) {
            break;  // 缓冲区满了
        }
        ctx->pos++;
    }
    spin_unlock(&inode->lock);
    return 0;
}

/* VFS interface functions VFS接口函数 */

static struct super_operations template_fs_super_operation;
static struct dentry_operations template_fs_dentry_operation;
static struct inode_operations template_fs_inode_operation;

/*
 * Kill superblock (unmount)
 * 杀死superblock(卸载)
 * @param sb: Superblock to kill 要杀死的superblock
 */
static void template_fs_kill_sb(struct super_block * sb);

/*
 * Allocate an inode
 * 分配一个inode
 * @param sb: Superblock pointer superblock指针
 * @return: Allocated inode, or NULL on failure 成功返回分配的inode，失败返回NULL
 */
static struct inode *template_fs_alloc_inode(struct super_block *sb);

/*
 * Destroy an inode
 * 销毁一个inode
 * @param node: Inode to destroy 要销毁的inode
 */
static void template_fs_destroy_inode(struct inode * node);

/*
 * Get filesystem statistics
 * 获取文件系统统计信息
 * @param dentry: Dentry pointer dentry指针
 * @param star: Statistics structure 统计信息结构体
 * @return: 0 on success 成功返回0
 */
static int template_fs_statfs(struct dentry * dentry, struct kstatfs *star);

/*
 * Get VFS dentry from template_fs dentry
 * 从template_fs dentry获取VFS dentry
 * @param vdentry: VFS dentry VFS dentry
 * @param dentry: template_fs dentry template_fs dentry
 * @return: VFS dentry VFS dentry
 */
static struct dentry * template_fs_get_dentry(struct dentry * vdentry, struct template_fs_dentry *dentry);

/*
 * Get VFS superblock from template_fs superblock
 * 从template_fs superblock获取VFS superblock
 * @param sb: template_fs superblock template_fs superblock
 * @return: VFS superblock, or NULL on failure VFS superblock，失败返回NULL
 */
static struct super_block *template_fs_get_vfs_superblock(struct template_fs_superblock *sb);

/*
 * Get VFS inode from template_fs inode
 * 从template_fs inode获取VFS inode
 * @param template__inode: template_fs inode template_fs inode
 * @param sb: Superblock pointer superblock指针
 * @return: VFS inode, or NULL on failure VFS inode，失败返回NULL
 */
static struct inode * template_fs_get_inode(struct template_fs_inode *, struct super_block*);

/*
 * Mount operation
 * 挂载操作
 * @param fs_type: Filesystem type 文件系统类型
 * @param flags: Mount flags 挂载标志
 * @param template__name: Mount name 挂载名称
 * @param data: Mount data 挂载数据
 * @return: Root dentry, or error pointer 根dentry，或错误指针
 */
static struct dentry *template_fs_mount(struct file_system_type *fs_type,
                                      int flags,
                                      const char *template__name,
                                      void *data);

/* Filesystem type structure 文件系统类型结构体 */
static struct file_system_type fs_type = {
    .name     = "template_fs",
    .fs_flags = 0,
    .init_fs_context = NULL,
    .kill_sb  = template_fs_kill_sb,
    .mount    = template_fs_mount,
    .owner    = THIS_MODULE,
};

/*
 * Mount the filesystem
 * 挂载文件系统
 */
static struct dentry *template_fs_mount(struct file_system_type *fs_type,
                                      int flags,
                                      const char *template__name,
                                      void *data)
{
    // Create filesystem superblock 创建文件系统superblock
    struct template_fs_superblock *fs_superblock = template_fs_superblock_create();
    if (fs_superblock == NULL){
        return ERR_PTR(-ENOMEM);        
    }
    
    // Allocate root dentry 分配根dentry
    struct dentry * root = __d_alloc(NULL,"root");
    if(root == NULL){
        template_fs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);   
    }

    // Create template_fs dentry 创建template_fs dentry
    struct template_fs_dentry* dentry = kmalloc(sizeof(struct template_fs_dentry),GFP_KERNEL);
    if(dentry == NULL){
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    }
    
    // Initialize dentry 初始化dentry
    dentry->name = NULL;
    dentry->target_inode = fs_superblock->rootinode;
    dentry->magic = MAGIC;
    root->d_fsdata = dentry;
    
    // Get VFS inode 获取VFS inode
    struct inode * root_inode = template_fs_get_inode(dentry->target_inode,NULL);
    if(root_inode == NULL){
        kfree(dentry);
        template_fs_superblock_destory(fs_superblock);
        kfree(root);
        return ERR_PTR(-ENOMEM);        
    } 
    root->d_inode = root_inode;

    // Get VFS superblock 获取VFS superblock
    struct super_block *sb = template_fs_get_vfs_superblock(fs_superblock);
    if(sb == NULL){
        kfree(root);
        kfree(dentry);
        inode_put(root_inode);
        template_fs_superblock_destory(fs_superblock);
        return ERR_PTR(-ENOMEM);       
    }
    
    // Set up superblock and root dentry 设置superblock和根dentry
    sb->s_root = root;
    root->d_sb = sb;

    return root;
}

/* Superblock operations structure superblock操作结构体 */
static struct super_operations template_fs_super_operation = {
    .alloc_inode   = template_fs_alloc_inode,
    .destroy_inode = template_fs_destroy_inode, 
    .free_inode    = NULL,         
    .dirty_inode   = NULL,
    .write_inode   = NULL,
    .drop_inode    = NULL,
    .put_super     = template_fs_kill_sb,
    .sync_fs       = NULL,
    .statfs        = template_fs_statfs,
};

/*
 * Get VFS superblock from template_fs superblock
 * 从template_fs superblock获取VFS superblock
 */
static struct super_block *template_fs_get_vfs_superblock(struct template_fs_superblock *sb)
{
    if(sb == NULL) return NULL;
    
    // Allocate VFS superblock 分配VFS superblock
    struct super_block * vsb = alloc_super(NULL);
    if(vsb == NULL) return NULL;
    
    // Set up VFS superblock 设置VFS superblock
    vsb->s_fs_info = sb;
    vsb->s_d_op    = &template_fs_dentry_operation;
    vsb->s_op      = &template_fs_super_operation;    
    
    return vsb;
}

/*
 * Kill superblock (unmount)
 * 杀死superblock(卸载)
 */
static void template_fs_kill_sb(struct super_block * sb){
    put_super(sb);
}

/*
 * Get VFS inode from template_fs inode
 * 从template_fs inode获取VFS inode
 */
static struct inode * template_fs_get_inode(struct template_fs_inode * template__node,
                                          struct super_block* sb)
{
    if(template__node == NULL) return NULL;
    
    // Allocate new VFS inode 分配新的VFS inode
    struct inode* inode = new_inode(sb);
    if(inode == NULL) return NULL;  
    
    // Set up VFS inode 设置VFS inode
    inode->i_mode    = template__node->i_mode;
    inode->i_fop     = template__node->i_fop;
    inode->i_op      = &template_fs_inode_operation;
    inode->i_private = template__node;
    
    return inode;
}

/*
 * Allocate an inode
 * 分配一个inode
 */
static struct inode *template_fs_alloc_inode(struct super_block *sb)
{
    struct template_fs_superblock *supb = sb->s_fs_info;
    if(supb->magic != MAGIC) return NULL;
    
    // Create template_fs inode 创建template_fs inode
    struct template_fs_inode * inode = template_fs_create_empty_inode(supb,&template_fs_file_fops,10);
    if(inode == NULL) return NULL;
    
    // Get VFS inode 获取VFS inode
    struct inode *out_inode = template_fs_get_inode(inode,sb);
    if(out_inode == NULL){
        kfree(inode);
        return NULL;
    }
    
    return out_inode;
}

/*
 * Destroy an inode
 * 销毁一个inode
 */
static void template_fs_destroy_inode(struct inode * node)
{
    if(node == NULL) return;
    if(node->i_private != NULL) {
        struct template_fs_inode * template__node = node->i_private;
        list_del(&template__node->list_node);
        kfree(template__node);        
    }
}

/*
 * Get filesystem statistics
 * 获取文件系统统计信息
 */
static int template_fs_statfs(struct dentry * dentry, struct kstatfs *star){
    struct template_fs_dentry * template_fs_dentry = dentry->d_fsdata;
    struct template_fs_inode * template_fs_node = template_fs_dentry->target_inode;
    
    // Fill in statistics 填充统计信息
    star->f_type = MAGIC;
    star->f_bsize  = 0;
    star->f_blocks = 1;
    star->f_bfree  = 0;
    star->f_bavail = 0;
    star->f_files  = 128;
    star->f_ffree  = 127;
    
    return 0;
}

/* Inode operations inode操作 */

static int template_fs_create(struct mnt_idmap * map, struct inode * dir, struct dentry * dentry, umode_t mode, bool bool);
static struct dentry *template_fs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags);
static int template_fs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry);
static int template_fs_unlink(struct inode *dir, struct dentry *dentry);
static int template_fs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode);
static int template_fs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr);
static int template_fs_getattr(struct mnt_idmap *dmp, const struct path * path, struct kstat *stat, u32 u, unsigned int i);
static int template_fs_rmdir(struct inode *inode, struct dentry *dentry);

/* Inode operations structure inode操作结构体 */
static struct inode_operations template_fs_inode_operation = {
    .create = template_fs_create,
    .lookup = template_fs_vfs_lookup,
    .link   = template_fs_link,
    .unlink = template_fs_unlink,
    .mkdir  = template_fs_mkdir,
    .rmdir  = template_fs_rmdir,
    .setattr = template_fs_setattr,
    .getattr = template_fs_getattr
};

/*
 * Get VFS dentry from template_fs dentry
 * 从template_fs dentry获取VFS dentry
 */
static struct dentry * template_fs_get_dentry(struct dentry * vdentry, struct template_fs_dentry *dentry)
{
    if(dentry == NULL || vdentry == NULL) return NULL;
    vdentry->d_fsdata = dentry;
    return vdentry;
}

/*
 * Create a file (VFS operation)
 * 创建一个文件(VFS操作)
 */
static int template_fs_create(struct mnt_idmap * map, struct inode * dir, struct dentry * dentry, umode_t mode, bool bool)
{
    char *file_name = d_getname(dentry);

    struct template_fs_inode *dinode = dir->i_private;
    struct template_fs_dentry* file = template_fs_lookup(dinode,file_name);
    if(file == NULL){
        file = template_fs_create_file(dinode->sb,dinode,&template_fs_file_fops,10,file_name);
    }
    if (file == NULL){                               // Check if file was created 确定文件是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    if(template_fs_get_dentry(dentry,file) == NULL){ // Build dentry 构建dentry    
        return -1;
    }  

    struct inode * new_file_inode = template_fs_get_inode(file->target_inode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); // Notify filesystem to add to cache and bind resources 通知文件系统加入缓存，并进行资源绑定
    return 0;
} 

/*
 * Lookup a dentry (VFS operation)
 * 查找一个dentry(VFS操作)
 */
static struct dentry *template_fs_vfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags){
    struct template_fs_inode *dir_inode = (struct template_fs_inode *)dir->i_private; // Get filesystem inode 获取文件系统本身的inode
    char *name = dentry->d_name.name;                           
    struct template_fs_dentry *pos;    
    pos = template_fs_lookup(dir_inode,name);       // Lookup dentry 查询获得dentry
    if(pos == NULL) return NULL;

    if(template_fs_get_dentry(dentry,pos) == NULL)
        return NULL;

    struct inode * find_inode = template_fs_get_inode(pos->target_inode,dir->i_sb);
    if(find_inode == NULL)
        return NULL;
    
    return d_add(dentry,find_inode);
    return NULL;
}

/*
 * Create a hard link (VFS operation)
 * 创建一个硬链接(VFS操作)
 */
static int template_fs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry)
{
    struct template_fs_inode *old_inode = (struct template_fs_inode *)old_dentry->d_inode->i_private;
    struct template_fs_inode *dir_inode = (struct template_fs_inode *)dir->i_private;
    
    // Create new dentry pointing to same inode 创建指向相同inode的新dentry
    struct template_fs_dentry *new_dentry = template_fs_create_dentry(dir_inode, old_inode, dentry->d_name.name);
    if (new_dentry == NULL) {
        return -ENOMEM;
    }
    
    template_fs_get_dentry(dentry,new_dentry);
    dentry->d_sb = dir->i_sb;
    d_add(dentry,dentry->d_inode);
    return 0;
}

/*
 * Unlink a file (VFS operation)
 * 取消文件链接(VFS操作)
 */
static int template_fs_unlink(struct inode *dir, struct dentry *dentry)
{
    struct template_fs_inode *parent_inode = (struct template_fs_inode *)dir->i_private;
    struct template_fs_dentry *delete_dentry = (struct template_fs_dentry *)dentry->d_fsdata;
    template_fs_delete_dentry(parent_inode->sb, parent_inode, delete_dentry);
    return 0;
}

/*
 * Create a directory (VFS operation)
 * 创建一个目录(VFS操作)
 */
static int template_fs_mkdir(struct mnt_idmap *mnt, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    char *file_name = d_getname(dentry);

    struct template_fs_inode *dinode = dir->i_private;
    struct template_fs_dentry* file = template_fs_lookup(dinode,file_name);
    if(file == NULL){
        file = template_fs_create_dir(dinode->sb,dinode,file_name);
    }
    if (file == NULL){                               // Check if directory was created 确定目录是否被创建
        return -1;
    }
    file->target_inode->i_mode = S_IFREG | (mode & 0777);
    
    if(template_fs_get_dentry(dentry,file) == NULL){ // Build dentry 构建dentry    
        return -1;
    }  
    struct inode * new_file_inode = template_fs_get_inode(dinode,dir->i_sb);
    if(new_file_inode == NULL)
        return -1;

    d_add(dentry,new_file_inode); // Notify filesystem to add to cache and bind resources 通知文件系统加入缓存，并进行资源绑定
    return 0;
}

/*
 * Release a dentry (VFS operation)
 * 释放一个dentry(VFS操作)
 */
static int template_fs_release_dentry(struct dentry *dentry){
    struct template_fs_dentry *fs_dentry = dentry->d_fsdata;
    dentry_put(fs_dentry->target_inode->sb, fs_dentry);
}

/*
 * Remove a directory (VFS operation)
 * 删除一个目录(VFS操作)
 */
static int template_fs_rmdir(struct inode *dir, struct dentry *dentry){
    template_fs_release_dentry(dentry);
    return simple_unlink(dir,dentry);
}

/*
 * Set attributes (VFS operation)
 * 设置属性(VFS操作)
 */
static int template_fs_setattr(struct mnt_idmap *map, struct dentry *dentry, struct iattr *iattr)
{
    struct template_fs_dentry* file = dentry->d_fsdata;
    file->target_inode->i_mode = (iattr->ia_mode & S_IFMT) | iattr->ia_mode;
    return 0;
}

/*
 * Get attributes (VFS operation)
 * 获取属性(VFS操作)
 */
static int template_fs_getattr(struct mnt_idmap *dmp, const struct path * path, struct kstat *stat, u32 u, unsigned int i)
{
    struct dentry *d = path->dentry;
    struct template_fs_dentry* file = d->d_fsdata;
    struct template_fs_inode *inode;

    if (!file || !(inode = file->target_inode))
        return -ENOENT;
    
    // Fill in stat structure 填充stat结构体
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

/* Dentry operations structure dentry操作结构体 */
static struct dentry_operations template_fs_dentry_operation = {
    .d_release = template_fs_release_dentry,
};

/*
 * Initialize template filesystem
 * 初始化模板文件系统
 */
static int __init template_fs_ops_init(void){
    register_filesystem(&fs_type);
    return 0;
}

fs_initcall(template_fs_ops_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("你");
MODULE_DESCRIPTION("简易设备文件系统 template_fs 测试");







