
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
    file_close(pipe_a,0);


    remove_dentry("/test_file");

     pipe_a = filp_open("/test_file", O_RDWR,0755);
    if(IS_ERR(pipe_a))
        pr_info("file has bean remove\n");
    else
        pr_info("file is not remove\n");

}