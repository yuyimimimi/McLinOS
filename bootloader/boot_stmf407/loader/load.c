#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_hal.h" 

#include "../../../../../include/generated/autoconf.h"
#define KERNEL_START_ADDRESS CONFIG_LD_FLASH_START_ADDRESS


void _send_byte(char byte);


void print_GCC_Message()
{
    printf("\r\n\r\nGCC Version: %d.%d.%d Compiler:%s ", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__,__VERSION__);    
    #if defined(__x86_64__) || defined(__aarch64__)
        printf("64-bit target\r\n");
    #else
        printf("32-bit target\r\n");
    #endif
}

void jmp_to_start(uint32_t addr);
void load_kernel()
{
    print_GCC_Message();
    printf("hal test\r\n");
    jmp_to_start(KERNEL_START_ADDRESS);
}

void jmp_to_start(uint32_t addr)
{
    printf("jmp_to_start addr: 0x%08x\r\n", addr);

    uint32_t jmp_addr =  addr; 
    uint32_t msp_value = *(volatile uint32_t*)jmp_addr;
    uint32_t start_addr = *(volatile uint32_t*)(jmp_addr+4) ;

    printf("Application MSP: 0x%08x\r\n", msp_value);
    printf("Application Entry Point: 0x%08x\r\n", start_addr);

    SCB->VTOR = addr;
    printf("Set Start address : 0x%08x\r\n", start_addr);
    __set_MSP(msp_value);   
    
    int (*kernel_main)(void*) = (int (*)(void*))(start_addr);

    printf("Jump to Second Boot!\r\n");
    kernel_main(_send_byte);

    printf("if you get this message,you must got a bug!,check your code!\r\n");      
    while(1);
}

