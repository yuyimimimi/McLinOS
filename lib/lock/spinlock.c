#include <linux/irqflags.h>
#include <linux/spinlock.h>

int __weak try_atomic_set_flag(int* lock, int flag);
int __weak try_atomic_set_flag(int* lock, int flag)
{
    local_irq_disable();
    int ret = -1;
    if (*lock != flag) {
        *lock = flag;
        ret = 0;
    }
    local_irq_enable();
    return ret;
}


void _spin_lock(spinlock_t* lock)
{
    while (1)
    {
        if(try_atomic_set_flag(&lock->flag,1) == 0){
            lock->owner = get_current_task();
            break;
        }
        else if(lock->owner == get_current_task()){  //如果已经被锁住但是锁是自己的，直接返回(正常不会出现这种情况)
            return;   
        }
        else{
            __delay(5); //主动让出时间片
        }
    }
}


void _spin_unlock(spinlock_t* lock)
{
    try_atomic_set_flag(&lock->flag,0);
}