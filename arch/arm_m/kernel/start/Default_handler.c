#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/irqflags.h>
#include <generated/autoconf.h>

extern unsigned int _estack;
extern unsigned int _sdata;
extern unsigned int _edata;
extern unsigned int _sidata;
extern unsigned int _sbss;
extern unsigned int _ebss;

typedef void(*_put_char_t)(char c);
_put_char_t _put_char = NULL;

struct export_node_struct* __boot_export_table_start;
struct export_node_struct* __boot_export_table_end;

void* find_symbol_by_fnname_from_bootloader(char *name);



void Reset_Handler(struct export_node_struct* start,struct export_node_struct* end,uint32_t other)
{
    __asm volatile ("ldr sp, =_estack");
    unsigned int *src = &_sidata;
    unsigned int *dest = &_sdata;
    unsigned int size = (unsigned int)(&_edata - &_sdata);
    for (unsigned int i = 0; i < size; i++) {
        dest[i] = src[i];
    }
    unsigned int *bss_start = &_sbss;
    unsigned int *bss_end = &_ebss;
    size = (unsigned int)(bss_end - bss_start);
    for (unsigned int i = 0; i < size; i++) {
        bss_start[i] = 0;
    }
    
    __boot_export_table_start= start;
    __boot_export_table_end = end;

    _put_char = find_symbol_by_fnname_from_bootloader("putchar");
    Kernel_Start();
    while(1);
}


void IRQ_Generic_Handler(void);

void __weak NMI_Handler(void){
     printk(KERN_INFO "You have not set NMI_Handler\n");
     while (1){}
}
void __weak HardFault_Handler(void){
     printk(KERN_INFO "You have not set HardFault_Handler\n");
     while (1){}
}
void __weak MemManage_Handler(void){
     printk(KERN_INFO "You have not set MemManage_Handler\n");
     while (1){}
}
void __weak BusFault_Handler(void){
     printk(KERN_INFO "You have not set BusFault_Handler\n");
     while (1){}
}
void __weak UsageFault_Handler(void){
     printk(KERN_INFO "You have not set UsageFault_Handler\n");
     while (1){}
}
void __weak SVC_Handler(void){
     printk(KERN_INFO "You have not set SVC_Handler\n");
     while (1){}
}
void __weak DebugMon_Handler(void){
     printk(KERN_INFO "You have not set DebugMon_Handler\n");
    while (1){}
}
void __weak PendSV_Handler(void){
     printk(KERN_INFO "You have not set PendSV_Handler\n");
     while (1){}
}
void __weak SysTick_Handler(void){
     printk(KERN_INFO "You have not set SysTick_Handler\n");
     while (1){}
}
void __weak Unknow_Handler(void){
     printk(KERN_INFO "Unknow Interrupte\n");
     while (1){}
}

void* __attribute__((__section__(".isr_vector"))) g_pfnVectors[] = {
    (void *)&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    Unknow_Handler,
    Unknow_Handler,
    Unknow_Handler,
    Unknow_Handler,
    SVC_Handler,
    DebugMon_Handler,
    Unknow_Handler,
    PendSV_Handler,
    SysTick_Handler,
    #include "Interruptvectorscale.h"
};


