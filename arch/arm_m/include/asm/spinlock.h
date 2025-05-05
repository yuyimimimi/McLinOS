#ifndef __SPIN_LOCK_H_
#define __SPIN_LOCK_H_

#include <linux/sched.h>

typedef struct { 
     int flag; 
} arch_spinlock_t;

static int __spin_init(arch_spinlock_t *lock) 
{
    lock->flag = 0;
}

static int __spin_lock(arch_spinlock_t *lock) {

    stop_all_scheduler();
    if(lock->flag == 0){
          lock->flag = 1;  
          start_all_scheduler();
        return 1;
    }
    else
    {
        start_all_scheduler();
        return 0;
    }
}

static void  __spin_unlock( arch_spinlock_t *lock ){
    lock->flag = 0;
}




#endif // 1