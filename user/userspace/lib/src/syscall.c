#include <stdlib.h>

struct systemcall {
    int r3;
    int r4;
    int r5;
    int r7;
}__attribute__((packed));

int __system_call(int sys_no, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5)
{
    register int r0 __asm("r0") = arg0;
    register int r1 __asm("r1") = arg1;
    register int r2 __asm("r2") = arg2;

    struct systemcall syscall_argvs = {
        .r3 = arg3,
        .r4 = arg4,
        .r5 = arg5,
        .r7 = sys_no,
    };
    register int r3 __asm("r3") = &syscall_argvs;

    __asm__ volatile (
        "svc 0"  
        : "=r" (r0)
        : "r" (r0), "r" (r1), "r" (r2), "r" (r3)
        : "memory"
    );
    return r0;
}


