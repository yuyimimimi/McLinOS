#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/sched.h>



static void spin_lock_init(spinlock_t* lock){
    __spin_init(&lock->rlock.raw_lock);
}

static void spin_lock(spinlock_t* lock)
{  
    while (1)
    {
        if(__spin_lock(&lock->rlock.raw_lock) == 1){
            lock->owner = get_current_task();
            break;
        }
        else  if(lock->owner == get_current_task()){  //如果已经被锁住但是锁是自己的，直接返回(正常不会出现这种情况)
            return;   
        }
        else{
            __delay(5); //主动让出时间片
        }
    }
}

static void spin_unlock(spinlock_t* lock)
{
    __spin_unlock(&lock->rlock.raw_lock);
}

#endif // !__SPINLOCK_H__