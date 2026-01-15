#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/libfdt.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/loader.h>
#include <linux/irqflags.h>
#include <generated/autoconf.h>
#include "main.h"

extern void  arch_init(void);
extern size_t get_global_heap_size(void);
extern int printk_service_init(void);
extern int  main_tty_dev_init(void);
extern void base_out_opt_device_init(void);
extern void SysTick_init(void);
extern void sys_expand_heap_init(void);
extern void sys_heap_init(void);
extern void IRQ_int_router(void);
int __get_task_using_cpu_core();




void print_GCC_Message()
{
    pr_info("gcc version %d.%d.%d (%sGCC Compiler:%s )# %s %s %s %s\n",
        __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, PREFIX,__VERSION__,
        COMPILE_TIME,PLATFORM,STRUCT,
        #if defined(__x86_64__) || defined(__aarch64__)
        "64-bit target "
        #else
        "32-bit target "
        #endif
        );
    pr_info("RAM:0x%0x - 0x%0x FLASH:0x%0x - 0x%0x  \n",
            CONFIG_LD_RAM_START_ADDRESS,CONFIG_LD_RAM_START_ADDRESS+CONFIG_LD_KERNEL_USE_RAM_SIZE*1024
            ,CONFIG_LD_FLASH_START_ADDRESS,CONFIG_LD_FLASH_START_ADDRESS+CONFIG_LD_KERNEL_USE_FLASH_SIZE*1024
        );
    pr_info("Start from core:%d \n",__get_task_using_cpu_core());
}

static void device_tree_init(){
    if((&__dtb_file_start_address[0] == &__dtb_file_end_address[0]) || (fdt_check_header(__dtb_file_start_address)!= 0))
    {while(1){}}
}

static void memory_init(void){
    sys_heap_init();
    sys_expand_heap_init();
}

static  void printk_init()
{
    printk_service_init();
    printk(KERN_INFO "printk init\n");
}

int test_task(void)
{
    return 1;
}


int Init_Kernel(void* argv)
{

    do_archinitcall();

    do_arch_initcall_sync();
    do_subsysinitcall();
    do_subsys_initcall_sync();
    
    do_deviceinitcall();
    do_device_initcall_sync();
    
    do_lateinitcall();
    do_late_initcall_sync();

    return 0;
}



int Kernel_Start(void)     
{
    local_irq_disable();
    
    print_GCC_Message();

    IRQ_int_router();

    do_initcallearly();
    do_pureinitcall();

    device_tree_init();
    memory_init();  
    
    arch_init();
  
    
    local_irq_enable();

    base_out_opt_device_init();  
    SysTick_init();

    do_fsinitcall();
    do_fs_initcall_sync();
    
    rootfs_init();

    main_tty_dev_init(); 
    printk_init();    

    do_coreinitcall();  
    do_core_initcall_sync();
    
    kthread_run(Init_Kernel,NULL,"init");

    do_postcoreinitcall();

    msp_init();

    do_postcore_initcall_sync();

    return 0; //never return 
}

