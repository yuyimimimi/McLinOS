#include <linux/kernel.h>
#include <generated/autoconf.h>
#include <linux/spinlock.h>



#ifndef _u
#ifdef __ASSEMBLER__
#define _u(x) x
#else
#define _u(x) x ## u
#endif
#endif

#define SIO_CPUID_OFFSET _u(0x00000000)
#define SIO_BASE         _u(0xd0000000)

static uint32_t get_core_num(void) {
    return (*(uint32_t *) (SIO_BASE + SIO_CPUID_OFFSET));
}
int __get_task_using_cpu_core(){
    return get_core_num();
}

static uint32_t irq_numbers_core0;
static uint32_t irq_numbers_core1;
static uint32_t irq_number_core0 = 0;
static uint32_t irq_number_core1 = 0;

static void restore_interrupts(uint32_t status) {
    __asm volatile (
        ".syntax unified\n"
        " msr PRIMASK, %0"
        : 
        : "r" (status) 
        : "memory"
    );
}

static uint32_t save_and_disable_interrupts(void) {
    uint32_t status;
    __asm volatile (
        ".syntax unified\n"
        " mrs %0, PRIMASK\n"
        " cpsid i" 
        : "=r" (status) 
        : 
        : "memory"
    );
    return status;
}


void __enable_irq() 
{
    uint32_t core_number =  get_core_num();
    uint32_t* irq_numbers;
    uint32_t* irq_number;
    if(core_number == 0){
      irq_numbers = &irq_numbers_core0;
      irq_number  = &irq_number_core0;
    }
    else {
      irq_numbers = &irq_numbers_core1;
      irq_number  = &irq_number_core1;  
    }
    if(irq_number[0] <= 0)return;
    if(irq_number[0] > 1){
      irq_number[0] --;
      return;
    }
    restore_interrupts(irq_numbers[irq_number[0]]);
    irq_number[0]--; 
}

void __disable_irq() 
{
    uint32_t core_number =  get_core_num();
    uint32_t* irq_numbers;
    uint32_t* irq_number;
    if(core_number == 0){
      irq_numbers = &irq_numbers_core0;
      irq_number  = &irq_number_core0;
    }
    else {
      irq_numbers = &irq_numbers_core1;
      irq_number  = &irq_number_core1;  
    }
    if(irq_number[0] >=  1){
       irq_number[0] ++;
       return;
    }
    else 
    {
      irq_numbers[0] = save_and_disable_interrupts();
      irq_number[0]++;
      return;      
    }
}

