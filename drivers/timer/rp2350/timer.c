#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>





uint64_t time_us_64() {
    volatile uint32_t *timer_base = (volatile uint32_t *)0x400b0000u;
    uint32_t hi, lo;
    do {
        hi = timer_base[0x24 / 4];  // TIMERAWH
        lo = timer_base[0x28 / 4];  // TIMERAWL
    } while (timer_base[0x24 / 4] != hi);  // 检查高32位是否变化
    return ((uint64_t)hi << 32) | lo;
}

time64_t  ktime_get(){
    return time_us_64();
}

