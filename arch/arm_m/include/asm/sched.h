#ifndef __SCHED_H__
#define __SCHED_H__                

#include <linux/types.h>

typedef struct {
    uint32_t psp;
} __TaskContext;         


extern void change_to_task_mode(void)  __attribute__((naked));  
extern void __sched(void)              __attribute__((naked)); 

#define TaskContext                     __TaskContext



#endif /* __CONTEXT_H__ */
