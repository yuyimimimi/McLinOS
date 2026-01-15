#include <linux/kernel.h>
#include <linux/types.h>


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
    uint32_t lr;   
    uint32_t pc;    
    uint32_t psr; 
    uint32_t msp;  
    uint32_t psp;  
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
        "TST LR, #4            \n"  
        "ITE EQ                 \n"
        "MRSEQ R0, MSP          \n"  
        "MRSNE R0, PSP          \n"  
        "B __hard_fault_handler_c \n" 
    );
}
