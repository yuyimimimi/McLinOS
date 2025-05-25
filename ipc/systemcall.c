#include <linux/kernel.h>
#include <linux/sys_call.h>
#include <asm/sched.h>
#include <linux/script.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>


extern void* find_table_by_fnname(char *name);

typedef int (*syscall_func_t)(void*, void*, void*, void*, void*, void*);

static syscall_func_t *syscall_table = NULL;
static uint32_t        syscall_number = 0;

int system_call(int sys_no,void* arg0,void* arg1,void* arg2,void* arg3,void* arg4,void* arg5)
{
    if(syscall_table == NULL){
        pr_err("syscall is not ready\n");
        return 0;
    }
    if(sys_no > syscall_number)
    {
       pr_err("unknown system call (%d)",sys_no);
       return 0;
    }
    syscall_func_t func = syscall_table[sys_no];
    if (!func) {
        pr_info("unimplemented system call (%d)\n", sys_no);
        return -ENOSYS;
    }
    return func(arg0, arg1, arg2, arg3, arg4, arg5);
}


static char* goto_next_data_line(char *text_data)
{
    char* index = text_data;
    index = goto_next_char(index,'\n','\0');//跳转到下级换行符
    if(index == NULL) return NULL;
    index = goto_next_string(index,'\0');
    return index;
}


struct syscall{
    char number[8];
    char ABI   [32];
    char Name  [32];
    char EntryPoint[32];
};


static int get_line_data(char *text_data ,struct syscall* syscall_data)
{
    int number = 0;
    char* index = text_data;
    index = goto_next_string(index,'\0');
    if(index == NULL) return -number;
    //printk(KERN_DEBUG "index:(%d):\n%s\n",number,index);
    number++;
    index += copy_string_data(index,syscall_data->number);
    index = goto_next_string(index,'\0');
    if(index == NULL) return -number;
    //printk(KERN_DEBUG "index:(%d):\n%s\n",number,index);
    number++;
    index += copy_string_data(index,syscall_data->ABI);
    index = goto_next_string(index,'\0');
    if(index == NULL) return -number;
    //printk(KERN_DEBUG "index:(%d):\n%s\n",number,index);
    number++;
    index += copy_string_data(index,syscall_data->Name);
    index = goto_next_string(index,'\0');
    if(index == NULL) return -number;
    //printk(KERN_DEBUG "index:(%d):\n%s\n",number,index);
    number++;
    index += copy_string_data(index,syscall_data->EntryPoint);
    index = goto_next_string(index,'\0');
    if(index == NULL) return -number;
    return 0;
}

static char* get_next_line_data(char* text_data,struct syscall*syscall_data)
{
    char *index = text_data;
    index = goto_next_data_line(index);
    //if(index != NULL)
    //printk(KERN_INFO "------------------------\nsyscall data:\n%s\n",index);
    int err = get_line_data(index, syscall_data);
    if( err == 0){
        //printk(KERN_INFO "get syscall data\n");
        return index;
    }
    //pr_err( "not get syscall data:%d\n",err);
    return NULL;
}

static uint32_t str_to_uint32(const char *s) {
    uint32_t result = 0;
    while (*s) {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}

static int get_syscall_numbers(char* text_data)
{
    char *index = text_data;
    struct syscall syscall_data;
    uint32_t number = 0;
    while (1)
    {
       index = get_next_line_data(index,&syscall_data);
       if(index == NULL){
            break;
       }
       else
       {
        uint32_t syscallnumber = str_to_uint32(syscall_data.number);
        if(syscallnumber > number)
        {
            number = syscallnumber;
        }
        //printk(KERN_INFO "\nsyscall number:%d\n",syscallnumber);
        }
    }
    return ++number;
}

static void load_syscall(struct syscall* syscall_data)
{
    int number = str_to_uint32(syscall_data->number);
    void *fn = find_table_by_fnname(syscall_data->EntryPoint);
    if(fn == NULL){
        pr_err("can not get funcation for systemcall:%s",syscall_data->EntryPoint);
        while (1){}
    }
    syscall_table[number] = fn;
}


static int parse_syscall_table(char *table_data)
{
    //pr_info(table_data);
    struct syscall syscall_data;
    char *index = table_data;
    syscall_number = get_syscall_numbers(table_data);
    if(syscall_number == 0){
        pr_err("no syscall dected\n");
        while(1){}
    }
    syscall_table = kmalloc(sizeof(syscall_func_t)*syscall_number,GFP_KERNEL);
    if(syscall_table == NULL){
        pr_err("can not alloc syscall table\n");
        while (1){}
    }
    memset(syscall_table,0,sizeof(syscall_func_t)*syscall_number);
    while (1)
    {
        index = get_next_line_data(index,&syscall_data);
        if(index == NULL)
           break;
        else  
        {
            load_syscall(&syscall_data);
        }
    }
    pr_info("syscall table read sucessful\n");
    return 0;
}





static int syscall_init()
{
    pr_info("system call init\n");

    struct file* f = filp_open("/boot/config.txt",O_RDWR,0);
    if(IS_ERR(f)){
        pr_err("can not open file:/boot/config.txt\n");
        while (1){}
    }

    int data_length = f->f_inode->i_size;
    char *file_data = kmalloc(data_length + 1,GFP_KERNEL);
    if (!file_data) {
        pr_err("cannot allocate memory for config\n");
        while(1){}                                   //系统调用是必须有的，所以如果出错拒绝启动
    }

    loff_t offset = 0;
    kernel_read(f,file_data,data_length,&offset);
    file_close(f,0);
    
    file_data[data_length] = '\0';
    preprocess_ini_data(file_data);

    char data[64];
    data[64] = '\0';
    char* index = get_value_from_ini(file_data,"syscall","SyscallTable",data);
    if(index == NULL){
        pr_err("can not get system call table file.please set it at file system\n");
        while (1){}                                           
    }
    kfree(file_data);
    

    f = filp_open(data,O_RDWR,0);
    if(IS_ERR(f)){
        pr_err("can not open file:%s",data);
        while (1){}                          
    }
    data_length = f->f_inode->i_size;
    file_data = kmalloc(data_length + 1,GFP_KERNEL);
    if (!file_data) {
        pr_err("cannot allocate memory for syscall table\n");
        while(1){}
    }
    offset = 0;
    kernel_read(f,file_data,data_length-1,&offset);
    file_close(f,0);

    file_data[data_length] = '\0';
    preprocess_ini_data(file_data);

    if (parse_syscall_table(file_data) != 0) {
        pr_err("failed to parse syscall table\n");
        while (1){}
    }
    kfree(file_data);

    printk(KERN_DEBUG "syscall table initialized with ( %u ) entries\n", syscall_number);    
    return 0;
}


core_initcall(syscall_init);












