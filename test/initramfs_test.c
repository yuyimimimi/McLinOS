
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

    
    ssize_t ret = 0;
    struct file * pipe_a = filp_open("/test_file",O_CREAT | O_RDWR,0755);
    loff_t offset;
    offset = 0;
    ret = kernel_write(pipe_a,"hello ramfs page\n",17,&offset);
    char test_buffer[20];
    offset = 0;
    ret = kernel_read(pipe_a,&test_buffer,17,&offset);
    pr_info("%s\n",test_buffer);  
    filp_close(pipe_a,0);


    remove_dentry("/test_file");

     pipe_a = filp_open("/test_file", O_RDWR,0755);
    if(IS_ERR(pipe_a))
        pr_info("file has bean remove\n");
    else
        pr_info("file is not remove\n");
    } 



    

static struct initramfs_dentry* initramfs_create_mem_file(
    struct initramfs_superblock *sb,struct initramfs_inode* parent_dentry_inode,struct file_operations* fop,uint32_t major,char *name,
    __file_data *file
    )
{
    struct initramfs_dentry* fsinode = initramfs_create_inode(sb,parent_dentry_inode,fop,major,name);
    fsinode->target_inode->i_mode = READONLY;
    fsinode->target_inode->size   = file->length;
    fsinode->target_inode->i_page = virt_to_page(file->data);
    return fsinode;
}


void print_file_head_data(__file_data *file)
{

    printk("file path:%s  length:%d mode:%d\n",file->path,file->length,file->mode);
    print_memory(file->data,file->length);
}
static void search_all_file(void){
    for (__file_data *file_head = __export_file_start; file_head != __export_file_end; file_head++) {
        print_file_head_data(file_head);
    }
    if(__export_file_start == __export_file_end)
    {
        printk("no any file\n");
    }
}

