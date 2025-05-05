#ifndef _KTHREADS_H_
#define _KTHREADS_H_

#include <linux/sched.h>
#include <linux/sprintf.h>
#include <linux/stdarg.h>
#include <generated/autoconf.h>

#define KTHREAD_DEFAULT_PRIORITY CONFIG_KTHREAD_DEFAULT_PRIORITY
#define KTHREAD_DEFAULT_STACK_SIZE 1024*CONFIG_KTHREAD_DEFAULT_STACK_SIZE
#define default_core_number CONFIG_KTHREAD_DEFAULT_USE_CPU_CORE

static struct task_struct *kthread_run(
    int (*threadfn)(void *data), void *data, const char namefmt[],...)
{
    block_scheduler(NULL);
    char name[128];
    va_list args;
    va_start(args, namefmt);
    vsnprintf(name, sizeof(name), namefmt, args);
    va_end(args);
    struct task_struct *t = task_run(
    threadfn,KTHREAD_DEFAULT_STACK_SIZE,data,KTHREAD_DEFAULT_PRIORITY,name,default_core_number);
    run_scheduler(NULL);
    return t;
}
 




#endif

