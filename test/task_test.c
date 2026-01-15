#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>

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

static uint8_t data = 0;
static struct quest_lock shared_lock;
static struct task_struct *thread_a;
static struct task_struct *thread_b;


static uint8_t data_add(uint8_t* data)
{
    data[0] ++;
    return data[0];
}
static uint8_t data_sub(uint8_t* data)
{
    data[0] --;
    return data[0];
}

static int thread_a_fn(void *argv)
{   
    int i = 0;
    while (1){  
        i = quest_lock_a(&shared_lock,data_add,&data);
        printk(KERN_INFO " --lock test task:%s,data:%d\n", get_current_task()->comm,i);
    }
}

static int thread_b_fn(void *argv){   
    int i = 0;
    while (1)
    {
        i = quest_lock_b(&shared_lock,data_sub,&data);
        printk(KERN_INFO " --lock test task:%s,data:%d\n", get_current_task()->comm,i);
        msleep(100);
    }
}

static int __init task_test()
{
    quest_lock_init(&shared_lock);
    thread_a = kthread_run(thread_a_fn,NULL,"tese_a");
    thread_b = kthread_run(thread_b_fn,NULL,"tese_b");
}
static void __exit task_exit()
{
    return;
}



module_init(task_test);
module_exit(task_exit);
