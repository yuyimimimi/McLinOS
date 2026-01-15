#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time64.h>
#include <generated/autoconf.h>
#include <linux/sched.h>

void call_mulit_core_scheduler(int cpu_number);
struct scheduler *get_scheduler_by_cpu_core_id(uint32_t core_id);


void sys_tick_hander(void) {
    for (size_t i = 0; i < CONFIG_CPU_NUM; i++)
    {
        struct scheduler * curren_scheduler = get_scheduler_by_cpu_core_id(i);
        if(curren_scheduler->s_flag != SCHEDULER_BLOCKED ){
            call_mulit_core_scheduler(i);
        }
    }
}
