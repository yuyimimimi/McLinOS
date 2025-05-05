#ifndef _LINUX_MUTEX_H
#define _LINUX_MUTEX_H

// #include <linux/kthread.h>
#include <linux/types.h>
#include <linux/spinlock.h>

// extern int          get_scheduler_state();

struct mutex {
//     struct task_struct *owner;      // 锁的拥有者
//     struct list_head    wait_list;  // 等待的任务队列
//     bool                locked;     // 锁的状态
    spinlock_t          mutex_lock;
};


extern int l_mutex_init(struct mutex *mutex);
extern int l_mutex_lock(struct mutex *mutex);
extern int l_mutex_unlock(struct mutex *mutex);
extern int l_mutex_trylock(struct mutex *mutex);


#define DEFINE_MUTEX(name) \
       struct mutex name  

static inline void mutex_init(struct mutex *mutex)
{
    //  l_mutex_init(mutex);
}
static inline void mutex_lock(struct mutex *mutex)
{     
      // if(get_scheduler_state())
      //       l_mutex_lock(mutex);
}
static inline void mutex_unlock(struct mutex *mutex)
{
      // if(get_scheduler_state())
      //       l_mutex_unlock(mutex);
}



#endif 