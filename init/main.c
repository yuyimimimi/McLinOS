#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/libfdt.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/gpio/gpio.h>
#include <linux/kthread.h>
#include <linux/loader.h>


extern void SystemClock_Config(void);
extern void uart1_device_init_function_from_dtb(void);
extern size_t get_global_heap_size(void);
extern int printk_service_init(void);
extern int  main_tty_dev_init(void);
extern void base_out_opt_device_init(void);
extern int gpio_subsystem_init(void);
extern void SysTick_init(void);
extern void sys_expand_heap_init(void);
extern void sys_heap_init(void);
extern void IRQ_int_router(void);

static void do_initcallearly(void){
    for (initcall_t *fn = __start_initcallearly; fn != __end_initcallearly; fn++) 
        (*fn)();
}
static void do_pureinitcall(void){
    for (initcall_t *fn = __start_pureinitcall; fn != __end_pureinitcall; fn++) 
        (*fn)();
}
static void do_coreinitcall(void){
    for (initcall_t *fn = __start_coreinitcall; fn != __end_coreinitcall; fn++) 
        (*fn)();
}
static void do_core_initcall_sync(void){
    for (initcall_t *fn = __start_core_initcall_sync; fn != __end_core_initcall_sync; fn++) 
        (*fn)();
}
static void do_postcoreinitcall(void){
    for (initcall_t *fn = __start_postcoreinitcall; fn != __end_postcoreinitcall; fn++) 
        (*fn)();
}
static void do_postcore_initcall_sync(void){
    for (initcall_t *fn = __start_postcore_initcall_sync; fn != __end_postcore_initcall_sync; fn++) 
        (*fn)();
}
static void do_archinitcall(void){
    for (initcall_t *fn = __start_archinitcall; fn != __end_archinitcall; fn++) 
        (*fn)();
}
static void do_arch_initcall_sync(void){
    for (initcall_t *fn = __start_arch_initcall_sync; fn != __end_arch_initcall_sync; fn++) 
        (*fn)();
}
static void do_subsysinitcall(void){
    for (initcall_t *fn = __start_subsysinitcall; fn != __end_subsysinitcall; fn++)
        (*fn)();
}
static void do_subsys_initcall_sync(void){
    for (initcall_t *fn = __start_subsys_initcall_sync; fn != __end_subsys_initcall_sync; fn++) 
        (*fn)();
}
static void do_fsinitcall(void){
    for (initcall_t *fn = __start_fsinitcall; fn != __end_fsinitcall; fn++) 
        (*fn)();
}
static void do_fs_initcall_sync(void){
    for (initcall_t *fn = __start_fs_initcall_sync; fn != __end_fs_initcall_sync; fn++) 
        (*fn)();
}
static void rootfs_init(void){
    for (initcall_t *fn = __start_rootfsinitcall; fn != __end_rootfsinitcall; fn++) 
        (*fn)();
}
initcall_fn(deviceinitcall);
static void do_device_initcall_sync(void){
    for (initcall_t *fn = __start_device_initcall_sync; fn != __end_device_initcall_sync; fn++) 
        (*fn)();
}
static void do_lateinitcall(void){
    for (initcall_t *fn = __start_lateinitcall; fn != __end_lateinitcall; fn++) 
        (*fn)();
}
static void do_late_initcall_sync(void){
    for (initcall_t *fn = __start_late_initcall_sync; fn != __end_late_initcall_sync; fn++) 
        (*fn)();
}


void print_GCC_Message(){
    pr_info("\n\rGCC Version: %d.%d.%d   Compiler:%s ", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__,__VERSION__);    
    #if defined(__x86_64__) || defined(__aarch64__)
    pr_info("64-bit target ");
    #else
    pr_info("32-bit target ");
    #endif
	pr_info("Version: protype N/A\r\n");    
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



int Kernel_Init(void* argv)
{

    do_archinitcall();
    do_arch_initcall_sync();

    do_subsysinitcall();
    do_subsys_initcall_sync();
    
    do_deviceinitcall();
    do_device_initcall_sync();
    
    do_lateinitcall();
    do_late_initcall_sync();

    printk(KERN_DEBUG "system free memory size:%d kb\n\r",get_global_heap_size()/1024);  
    return 0;
}



int Kernel_Start(void)     
{
    SystemClock_Config();

    do_initcallearly();
    do_pureinitcall();

    device_tree_init();
    memory_init();  
    IRQ_int_router();

    base_out_opt_device_init();  
    print_GCC_Message();
    
    pr_info("device tree init\n");
    SysTick_init();
    printk(KERN_DEBUG "system free memory size:%d kb\n\r",get_global_heap_size()/1024);  

    do_fsinitcall();
    do_fs_initcall_sync();
    rootfs_init();

    main_tty_dev_init(); 
    printk_init();    

    do_coreinitcall();  
    do_core_initcall_sync();
    
    kthread_run(Kernel_Init,NULL,"kernel_init");

    do_postcoreinitcall();
    do_postcore_initcall_sync();
 
    while(1){}
    return 0;
}

