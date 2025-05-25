#include <linux/export.h>

extern void Reset_Handler();
static sys_restart_syscall(){
    Reset_Handler();
}
EXPORT_SYMBOL(sys_restart_syscall);
