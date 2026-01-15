#include <linux/types.h>
#include <linux/irqflags.h>
#include <linux/symbl.h>
#include <asm-generic/barrier.h>



static uint32_t (*hard_spinlock_blocking)(volatile int8_t *lock) = NULL;
static void (*hard_spin_unlock)(volatile int8_t *lock, uint32_t saved_irq) = NULL;



void __hardful_spinlock_init(volatile int8_t *lock)
{
    hard_spinlock_blocking = find_symbol_by_fnname_from_bootloader("spin_lock_blocking");
    hard_spin_unlock = find_symbol_by_fnname_from_bootloader("spin_unlock");
}


#define SPINLOCK_ID 1

int try_atomic_set_flag(int* lock, int flag)
{
    uint32_t irq_flag = 0;
    if(hard_spinlock_blocking!= NULL)
    irq_flag = hard_spinlock_blocking(SPINLOCK_ID);
    barrier();
    int ret = -1;
    if (*lock != flag) {
        *lock = flag;
        ret = 0;
    }
    barrier();
    if(hard_spinlock_blocking!= NULL)
    hard_spin_unlock(SPINLOCK_ID,irq_flag);
    return ret;
}
