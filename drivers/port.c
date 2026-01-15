#include <linux/kernel.h>
#include <generated/autoconf.h>
int __weak main_tty_dev_init(void){
    return 0;
}
void __weak early_printk(const char *fmt, ...){}
void __weak base_out_opt_device_init(){}
void __weak SysTick_init(void){}
int __weak __get_task_using_cpu_core(void){ 
    return 0;
}
void __weak  arch_init(void) {}
void __weak  clean_interrupt_flag(uint32_t flag){}

#include <linux/time64.h>
#include <linux/sched.h>

unsigned int HZ = CONFIG_SYSTEM_TIMER_HZ;
time64_t __weak ktime_get(){return 0;}
void __weak msp_init(){}

void __weak call_mulit_core_scheduler(int cpu_number){
    if(cpu_number == 0)
    i_sched();
}