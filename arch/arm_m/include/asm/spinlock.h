#ifndef __SPIN_LOCK_H_
#define __SPIN_LOCK_H_

#include <linux/sched.h>

typedef struct { 
     int flag; 
} arch_spinlock_t;


#endif // 1