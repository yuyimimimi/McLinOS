#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/types.h>


struct systemcall {
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r7;
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

extern int system_call(int sys_no,void* arg0,void* arg1,void* arg2,void* arg3,void* arg4,void* arg5);

typedef struct {
    uint32_t r0;  
    uint32_t r1;  
    uint32_t r2; 
    uint32_t r3;  
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} ExceptionStackFrame;

void SVC_Handler_C(ExceptionStackFrame *frame)
{
    struct systemcall* syscall_argvs = frame->r3;
    
    frame->r0 = system_call(syscall_argvs->r7,
                            frame->r0,
                            frame->r1, 
                            frame->r2,
                            syscall_argvs->r3,
                            syscall_argvs->r4,
                            syscall_argvs->r5);            
}

void SVC_Handler(void)
{
    __asm__ volatile (
        "TST lr, #4\n"
        "ITE EQ\n"
        "MRSEQ r0, MSP\n"
        "MRSNE r0, PSP\n"
        "B SVC_Handler_C\n"
    );
}





