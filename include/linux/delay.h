#ifndef __DELAY_H_
#define __DELAY_H_

#include <linux/time.h>
#include <linux/sched.h>

static void msleep(uint32_t time){
    __delay(time*HZ/1000);
}

#endif