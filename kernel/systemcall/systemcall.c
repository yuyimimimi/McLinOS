#include <linux/kernel.h>
#include <linux/sys_call.h>
#include <asm/sched.h>


int system_call(int sys_no,void* arg0,void* arg1,void* arg2,void* arg3,void* arg4,void* arg5)
{
    switch (sys_no)
    {
    case  SC_PRINTK:
            printk("%s\n",arg0);
            return 666;
            break;
    case  SC_SCHEDULER:
            __sched();
            break;

    default:
        printk("unknown system call (%d)",sys_no);
        break;
    }
}






