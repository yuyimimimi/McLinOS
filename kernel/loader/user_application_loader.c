#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/error.h>
#include <linux/sched.h>
#include <linux/script.h>

typedef void(*__start)(void);
typedef struct {
    uint32_t magic;         
    uint32_t kernel_ver;
    uint32_t cmd;         
    uint32_t arch_flags;
    uint32_t stack_size;
    __start  start;    
    uint32_t got_start;
    uint32_t got_end;
    uint32_t program_size;
} AppMeta;

typedef struct 
{
    uint32_t magic;         
    uint32_t min_kernel_ver;
    uint32_t max_kernel_ver;
    uint32_t cmd;
    uint32_t max_stack_size;  
    uint32_t default_priority;
    uint32_t arch_flags;
    uint32_t max_app_size;
}loader_config_t;

static loader_config_t loder_config;


static void print_app_meta(const AppMeta *meta)
{
    pr_info("AppMeta Information:\n");
    pr_info("  magic: %d\n", meta->magic);
    pr_info("  kernel_ver: 0x%08X\n", meta->kernel_ver);
    pr_info("  cmd: 0x%08X\n", meta->cmd);
    pr_info("  arch_flags: 0x%08X\n", meta->arch_flags);
    pr_info("  stack_size: %u\n", meta->stack_size);
    pr_info("  start: %p\n", meta->start);
    pr_info("  got_start: 0x%08X\n", meta->got_start);
    pr_info("  got_end: 0x%08X\n", meta->got_end);
    pr_info("  program_size: %d kb\n", meta->program_size/1024);
}



static int check_app(AppMeta* app)
{
    if (app->magic != loder_config.magic) {
        pr_err("Magic mismatch: expected %d, got %d\n", 
               loder_config.magic, app->magic);
        return -EINVAL;
    }
    if (app->kernel_ver < loder_config.min_kernel_ver ||
        app->kernel_ver > loder_config.max_kernel_ver) {
        pr_err("Kernel version mismatch\n");
        return -EINVAL;
    }
    if (app->cmd != loder_config.cmd) {
        pr_err("User App xmd is not avilible\n");
        return -EINVAL;
    }
    if (app->arch_flags != loder_config.arch_flags) {
        pr_err("Architecture flags mismatch\n");
        return -EINVAL;
    }
    if(app->stack_size/1024 > loder_config.max_stack_size){
        pr_err("loader:no memory app need: %d kb ram.\n",app->stack_size);
        return -EINVAL;
    }
    if(app->program_size > loder_config.max_app_size*1024){
        pr_err("this aplication:is too large need:%d kb ram to load\n",app->program_size/1024);
        return -EINVAL;
    }
    return 0;
}


static void set_got_table(AppMeta* app,char* app_data)
{
    uint32_t* got_start = (uint32_t*)((char*)app->got_start + (uint32_t)app_data);
    uint32_t* got_end = (uint32_t*)((char*)app->got_end + (uint32_t)app_data);
    if(got_start == got_end){
        printk(KERN_WARNING "loader: warning your application's got table is empty\n");
    }
    else
    {
        printk(KERN_DEBUG "got table start:0x%x end:0x%x \n",got_start,got_end);
    }

    for (uint32_t* entry = got_start; entry < got_end; entry++) {
        if (*entry != 0) 
        { // 仅处理非零条目（零表示未初始化的 GOT 条目）
            *entry += (uint32_t)app_data; // 将基地址加到 GOT 条目
        }
    }
}

static void print_AppMeta(AppMeta* app)
{
     printk(KERN_DEBUG "app_data: %d, %d, %d,%d, %d\n",
    app->magic,app->kernel_ver,app->cmd,app->arch_flags,app->stack_size/1024);
}


void print_haper_status(void);

int loader_user_app(char* file_path,void* argv)
{
    pr_info("load user app from file:%s\n",file_path);
    struct file * app_file = filp_open(file_path,O_RDWR,0755);
    if(IS_ERR(app_file)){
        pr_err("can not open file:%s\n",file_path);
        return PTR_ERR(app_file);
    }
    AppMeta load_app_data;
    loff_t offset = 0;
    kernel_read(app_file,&load_app_data,sizeof(load_app_data),&offset);
    //print_app_meta(&load_app_data);
    if(check_app(&load_app_data) != 0){
        file_close(app_file,0);
        return -1;
    }

    size_t app_size = load_app_data.program_size + 8;
    printk(KERN_DEBUG "system free memory size:%d kb\n\r",get_global_heap_size()/1024);  

    char* addr = kmalloc(app_size,GFP_NOWAIT); //此处使用复用的标识符，此时会强制使用主内存，保证兼容性
    if(addr == NULL){
        file_close(app_file,0);
        pr_err("user app loader: has not memory,need:%dkb\n",app_size/1024);
        return -ENOMEM;
    }

    printk(KERN_DEBUG "system free memory size:%d kb\n\r",get_global_heap_size()/1024);  

    offset = 0;
    kernel_read(app_file,addr,app_file->f_inode->i_size,&offset);    
    AppMeta* app_data_head = addr;
    void (*start)(void) = (uint32_t)(app_data_head->start) + (uint32_t)addr;
    printk(KERN_DEBUG"set start at     0x%x\n",start);
    printk(KERN_DEBUG"set base address 0x%x\n",addr);
    set_got_table(app_data_head,addr);
    struct task_struct  * t = task_run(start,app_data_head->stack_size,argv,loder_config.default_priority,file_path,0,(uint32_t)addr);
    if(IS_ERR(t)){
        kfree(addr);
    }
    return 0;
}



static uint32_t str_to_uint32(const char *s) {
    uint32_t result = 0;
    while (*s) {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}

static char *data = NULL;
static int loader_init()
{
    pr_info("kernel loader init\n");
    struct file* f = filp_open("/boot/config.txt",O_RDWR,0);
    if(IS_ERR(f)){
        pr_err("can not open file:/boot/config.txt\n");
        while (1){}
    }
    int data_length = f->f_inode->i_size;
    char *file_data = kmalloc(data_length + 1,GFP_KERNEL);
    if (!file_data) {
        pr_err("cannot allocate memory for config\n");
        while(1){}                                   //系统配置项必须有的，所以如果出错拒绝启动
    }
    loff_t offset = 0;
    kernel_read(f,file_data,data_length,&offset);
    file_close(f,0);

    file_data[data_length] = '\0';
    preprocess_ini_data(file_data);                //脚本预处理

    data = kmalloc(64,GFP_KERNEL);
    data[64] = '\0';

    char* index;
    int number = 0;

    number++;
    index = get_value_from_ini(file_data,"Loader_Config","magic",data);
    if(index == NULL) goto block_system;
    loder_config.magic = str_to_uint32(data);
    number++;
    index = get_value_from_ini(file_data,"Loader_Config","min_kernel_ver",data);
    if(index == NULL) goto block_system;
    loder_config.min_kernel_ver = str_to_uint32(data);
    number++;
    index = get_value_from_ini(file_data,"Loader_Config","max_kernel_ver",data);
    if(index == NULL) goto block_system;
    loder_config.max_kernel_ver = str_to_uint32(data);
    number++;
    index = get_value_from_ini(file_data,"Loader_Config","cmd",data);
    if(index == NULL) goto block_system;
    loder_config.cmd = str_to_uint32(data);
    number++;
    index = get_value_from_ini(file_data,"Loader_Config","max_stack_size",data);
    if(index == NULL) goto block_system;
    loder_config.max_stack_size = str_to_uint32(data);
    number++;
    index = get_value_from_ini(file_data,"Loader_Config","default_priority",data);
    if(index == NULL) goto block_system;
    loder_config.default_priority = str_to_uint32(data);
    number++;
    index = get_value_from_ini(file_data,"Loader_Config","arch_flags",data);
    if(index == NULL) goto block_system;
    loder_config.arch_flags = str_to_uint32(data);
    number++;
    index = get_value_from_ini(file_data,"Loader_Config","max_app_size",data);
    if(index == NULL) goto block_system;
    if(strcmp(data,"Auto") ==0)  loder_config.max_app_size = 0xffffffff;
    else  loder_config.max_app_size = str_to_uint32(data);
    number++;    
    index = get_value_from_ini(file_data,"user_start_app","path",data);
    if(index == NULL){
        pr_info("can not find user space start app\n");
    }
    kfree(file_data);
    return 0;

    block_system:
    pr_err("can not get loader config form /boot/config.txt(%d)\n",number);
    while (1){}     
    return -1;                                      
}

core_initcall(loader_init);

static char* argvs[] = {"start","goodmoring",NULL};

static int load_user_app()
{
    if(data[0]=='/')
    loader_user_app(data,argvs);
    kfree(data);
    return 0;
}
late_initcall(load_user_app);







