

static void test()
{
    struct file* f = filp_open("/boot/config.txt",O_RDWR,0);
    if(IS_ERR(f)){
        pr_err("can not open file:/boot/config.txt");
        return NULL;
    }

    int data_length = f->f_inode->i_size;
    char *file_data = kmalloc(data_length + 1,GFP_KERNEL);
    loff_t offset = 0;
    kernel_read(f,file_data,data_length,&offset);
    file_data[data_length] = '\0';

    preprocess_ini_data(file_data);


    char data[32];
    data[31] = '\0';
    char* index = get_value_from_ini(file_data,"TEST1","test_1",data);
    if(index != NULL)
    {
    pr_info("value: %s\n",data);    
    pr_info("data: %s\n",index);        
    }

    index = get_value_from_ini(index,NULL,"test_1_data1",data);
    if(index != NULL)
    {
        pr_info("value: %s\n",data);    
        pr_info("data: %s\n",index);        
    }
    
}


module_init(test);

