#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/types.h>

extern void C_IRQ_Dispatcher(int irq_number);
void IRQ_Generic_Handler(void)
{   
    int irq_number;
    __asm volatile (
        "MRS %0, IPSR\n\t"  
        "SUB %0, %0, #16\n\t"
        : "=r" (irq_number)
    );
    C_IRQ_Dispatcher(irq_number);
}



void __weak __enable_irq(){
    __asm volatile ("cpsie i": : : "memory");
} 

void __weak __disable_irq(){
    __asm volatile ("cpsid i": : : "memory");
}

void __enable_irq();
void __disable_irq();

extern void __user_error_process(void);
void UsageFault_Handler(){
    __user_error_process();
}

