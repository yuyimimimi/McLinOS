/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

# include <linux/cache.h>
# include <linux/math64.h>
# include <linux/time64.h>

extern time64_t ktime_get();

#define jiffies ktime_get()

extern unsigned int HZ;

static __always_inline timer_t ktime_get_ns(){
    return ktime_get();
}

static __always_inline time64_t ktime_get_real_seconds(){
    return ktime_get();  
}



#endif
