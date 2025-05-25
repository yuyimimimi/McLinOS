#include <linux/time64.h>
#include <generated/autoconf.h>
#include <linux/sched.h>

static time64_t time = 0;

unsigned int HZ = CONFIG_SYSTEM_TICK_FREQUENCY;

time64_t ktime_get(){
    return time;
}


void sys_tick_hander(void) {
    time++;
    i_sched();
}
