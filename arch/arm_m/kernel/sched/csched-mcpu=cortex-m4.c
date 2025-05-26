#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/sched.h>


extern void __default_Task_return_function(void);

void __init_Taskcontext(struct task_struct* task_data,uint32_t pic_offset){
    if(task_data == NULL) return;
    TaskContext* context = (TaskContext*)(&task_data->context);
    uint32_t* psp = (uint32_t*)(((uint32_t)task_data->stack_Top) & ~0x7) - 16;
    psp[15] = 0x01000000u;                  // xPSR
    psp[14] = task_data->entry;             // PC (任务入口地址)
    psp[13] = __default_Task_return_function;  // LR (返回地址)
    psp[12] = 0;                            // r12
    psp[11] = 0;                            // r3
    psp[10] = 0;                            // r2
    psp[9] = 0xFFFFFFFD;                    // r1     
    psp[8] = task_data->arg;                // r0
    psp[7] = 0;                     // r11
    psp[6] = 0;                     // r10
    psp[5] = pic_offset;            // r9
    psp[4] = 0;                     // r8
    psp[3] = 0;                     // r7
    psp[2] = 0;                     // r6
    psp[1] = 0;                     // r5
    psp[0] = (uint32_t)(psp);       // r4
    context->psp = (uint32_t)(psp);
} 

int __get_task_using_cpu_core(void){
    return 0;
}


extern void __user_error_process(void);
void UsageFault_Handler(){
    __user_error_process();
}



// 修正后的宏定义（显式声明输出变量）
#define __get_MSP()       ({ register uint32_t result; __asm volatile ("MRS %0, msp" : "=r" (result)); result; })
#define __get_PSP()       ({ register uint32_t result; __asm volatile ("MRS %0, psp" : "=r" (result)); result; })
#define __get_LR()        ({ register uint32_t result; __asm volatile ("MOV %0, lr"  : "=r" (result)); result; })
#define __get_PC()        ({ register uint32_t result; __asm volatile ("MOV %0, pc"  : "=r" (result)); result; })
#define __get_CONTROL()   ({ register uint32_t result; __asm volatile ("MRS %0, control" : "=r" (result)); result; })


typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;    // Link Register (LR)
    uint32_t pc;    // Program Counter (PC)
    uint32_t psr;   // Program Status Register (PSR)
    uint32_t msp;   // Main Stack Pointer (MSP)
    uint32_t psp;   // Process Stack Pointer (PSP)
} RegisterDump;

void get_register_dump(RegisterDump *dump, uint32_t *stack_frame) {
    dump->r0  = stack_frame[0];
    dump->r1  = stack_frame[1];
    dump->r2  = stack_frame[2];
    dump->r3  = stack_frame[3];
    dump->r12 = stack_frame[4];
    dump->lr  = stack_frame[5];
    dump->pc  = stack_frame[6];
    dump->psr = stack_frame[7];
    dump->msp = __get_MSP();
    dump->psp = __get_PSP();
}

extern void __kernel_error_process(void);
  
void __hard_fault_handler_c(uint32_t *stack_frame) {
    RegisterDump regs;
    get_register_dump(&regs, stack_frame);
    pr_err("\nHardFault Registers:\n");
    pr_err("R0:  0x%08x, R1:  0x%08x, R2:  0x%08x\n", regs.r0, regs.r1, regs.r2);
    pr_err("R3:  0x%08x, R12: 0x%08x, LR:  0x%08x\n", regs.r3, regs.r12, regs.lr);
    pr_err("PC:  0x%08x, PSR: 0x%08x\n", regs.pc, regs.psr);
    pr_err("MSP: 0x%08x, PSP: 0x%08x\n", regs.msp, regs.psp);
    __kernel_error_process();
}

void __attribute__((naked)) HardFault_Handler() 
{
    __asm__ volatile (
        "TST LR, #4            \n"  // 检查 LR 的 EXC_RETURN 位2（判断使用 MSP/PSP）
        "ITE EQ                 \n"
        "MRSEQ R0, MSP          \n"  // 如果 LR & 4 == 0，使用 MSP
        "MRSNE R0, PSP          \n"  // 否则使用 PSP
        "B __hard_fault_handler_c \n" // 跳转到 C 函数
    );
}