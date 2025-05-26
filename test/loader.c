
static void loader_user_app_test() 这是功能验证代码，已通过测试
{
    pr_info("test load user app\n");
    struct file * app_file = filp_open("/bin/main.bin",O_RDWR,0755);
    if(IS_ERR(app_file)){
        pr_err("can not open file\n");
        return;
    }
    size_t app_size = app_file->f_inode->i_size;
    char* addr = kmalloc(app_size + 8,GFP_NOWAIT); //此处使用复用的标识符，此时会强制使用主内存，保证兼容性
    if(addr == NULL){
        pr_err("user app loader: has not memory\n");
        return;
    }
    loff_t offset = 0;
    kernel_read(app_file,addr,app_size,&offset);

    AppMeta* app_data_head = addr;
    print_AppMeta(app_data_head);
    void (*start)(void) = (uint32_t)(app_data_head->start) + (uint32_t)addr;
    pr_info("start at     0x%x\n",start);
    pr_info("base address 0x%x\n",addr);
    set_got_table(app_data_head,addr);
    task_run(start,app_data_head->stank_size_hop,NULL,7,"test.bin",0,addr);
}

late_initcall(loader_user_app_test);






