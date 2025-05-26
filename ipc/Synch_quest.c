#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>



struct quest_lock {
    struct task_struct * task;
    spinlock_t lock;
    uint8_t key_1;
};

static void quest_lock_init(struct quest_lock *lock){
    spin_lock_init(&lock->lock);
    lock->key_1 = 0;
    lock->task = NULL;
}

static void* quest_lock_a(struct quest_lock *lock,void*(*fn)(void*),void *argv)
{
    while (1)
    {
        spin_lock(&lock->lock);
        if(lock->key_1 == 0){
            lock->key_1 = 1;
            void* data = fn(argv);
            spin_unlock(&lock->lock);
            return data;
        }
        spin_unlock(&lock->lock);
        msleep(10);
    }
}

static void* quest_lock_b(struct quest_lock *lock,void*(*fn)(void*),void *argv)
{
    while (1)
    {
        spin_lock(&lock->lock);
        if(lock->key_1 == 1){
            lock->key_1 = 0;
            void* data = fn(argv);
            spin_unlock(&lock->lock);
                    msleep(10);
            return data;
        }
        spin_unlock(&lock->lock);
        msleep(10);
    }
}


