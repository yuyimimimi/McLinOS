#include <linux/sched.h>
#include <linux/export.h>

static void sys_exit(){
    exit();
}
EXPORT_SYMBOL(sys_exit);



static void sys_sched_yield()
{
    i_sched();
}
EXPORT_SYMBOL(sys_sched_yield);

