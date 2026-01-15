#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/irqflags.h>
#include <linux/symbl.h>

extern void current_sched_start();
int i_sched();
static void (*hard_multicore_fifo_push_blocking)(uint32_t data) = NULL;
static int (*hard_multicore_doorbell_claim_unused)(uint core_mask, bool required) = NULL;
static void (*hard_multicore_doorbell_set_other_core)(uint doorbell_num) = NULL ;
static void (*hard_multicore_doorbell_clear_current_core)(uint doorbell_num) = NULL;
static uint (*hard_multicore_doorbell_irq_num)(uint doorbell_num) = NULL;


void multicore_fifo_push_blocking(uint32_t data){
    if(hard_multicore_fifo_push_blocking != NULL)
    hard_multicore_fifo_push_blocking(data);
    else{
        pr_err("can not find symbol:multicore_fifo_push_blocking\n");
    }
}


int multicore_doorbell_claim_unused(uint core_mask, bool required)
{
    if(hard_multicore_doorbell_claim_unused!= NULL)
        return hard_multicore_doorbell_claim_unused(core_mask,required);
    return -1;
}

void multicore_doorbell_set_other_core(uint doorbell_num) 
{
    if(hard_multicore_doorbell_set_other_core!= NULL)
        return hard_multicore_doorbell_set_other_core(doorbell_num);
}

void multicore_doorbell_clear_current_core(uint doorbell_num)
{
    if(hard_multicore_doorbell_clear_current_core!= NULL)
        return hard_multicore_doorbell_clear_current_core(doorbell_num);
}

static uint multicore_doorbell_irq_num(uint doorbell_num)
{
    if(hard_multicore_doorbell_irq_num != NULL)
        return hard_multicore_doorbell_irq_num(doorbell_num);
    return -1;
}



void Cpu1_sched_Handler()
{
    multicore_doorbell_clear_current_core(0);
    i_sched();
}

void call_mulit_core_scheduler(int cpu_number)
{
    if(cpu_number == 0)
    i_sched();
    if(cpu_number == 1)
    multicore_doorbell_set_other_core(0);
}



#define NUM_CORES 2u

static void cpu1_start_irq()
{
    pr_info("cpu1 start irq\n");
    uint32_t irq = multicore_doorbell_irq_num(0);
    pr_info("cpu1 start irq %d \n",irq);
    request_irq(irq,Cpu1_sched_Handler,NULL,"CPU1_Scheduler",NULL);   
    irq_set_enabled(irq, true);
}



int msp_init()
{
    pr_info("start cpu 1\n");
    
    hard_multicore_doorbell_claim_unused = find_symbol_by_fnname_from_bootloader("multicore_doorbell_claim_unused");
    hard_multicore_doorbell_set_other_core = find_symbol_by_fnname_from_bootloader("multicore_doorbell_set_other_core");
    hard_multicore_doorbell_clear_current_core = find_symbol_by_fnname_from_bootloader("multicore_doorbell_clear_current_core");
    hard_multicore_doorbell_irq_num = find_symbol_by_fnname_from_bootloader("multicore_doorbell_irq_num");
    hard_multicore_fifo_push_blocking = find_symbol_by_fnname_from_bootloader("multicore_fifo_push_blocking");

    multicore_fifo_push_blocking(cpu1_start_irq);    
    multicore_fifo_push_blocking(current_sched_start);
    for(int i =0 ;i <10000000;i++);
}





