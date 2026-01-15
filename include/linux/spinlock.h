#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <asm/irqflags.h>

struct task_struct;
struct task_struct* get_current_task(void);
void __delay(uint32_t time);



void _spin_lock(spinlock_t* lock);
void _spin_unlock(spinlock_t* lock);

static void spin_lock_init(spinlock_t* lock){
    lock->flag = 0;
}

static void spin_lock(spinlock_t* lock)
{  
    _spin_lock(lock);
}
static void spin_unlock(spinlock_t* lock)
{
    _spin_unlock(lock);
}


#endif // !__SPINLOCK_H__

