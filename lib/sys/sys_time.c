#include <linux/delay.h>
#include <linux/export.h>
void sys_nanosleep_time32(uint32_t time){
    msleep(time/1000);
}
EXPORT_SYMBOL(sys_nanosleep_time32);
