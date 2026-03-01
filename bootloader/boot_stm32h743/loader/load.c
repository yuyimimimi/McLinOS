#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32h743xx.h"

#include "../../../../../include/generated/autoconf.h"
#define KERNEL_START_ADDRESS CONFIG_LD_FLASH_START_ADDRESS

void _send_byte(char byte);

#define RAM_D1_START        ((uint32_t)0x24000000)
#define RAM_D1_SIZE         (512 * 1024)
#define DTCMRAM_START       ((uint32_t)0x20008000)
#define DTCMRAM_SIZE        (96 * 1024)
#define RAM_D2_START        ((uint32_t)0x30000000)
#define RAM_D2_SIZE         (288 * 1024)
#define ITCMRAM_START       ((uint32_t)0x00000000)
#define ITCMRAM_SIZE        (64 * 1024)
#define RAM_D3_START        ((uint32_t)0x38000000)
#define RAM_D3_SIZE         (64 * 1024)

#define SDRAM_START_ADDR    ((uint32_t)0xC0000000)
#define SDRAM_SIZE_BYTES    (32 * 1024 * 1024) // 32MB

// --- 函数定义 ---

// 初始化 DWT 计数器（仅支持 Cortex-M7）
static inline void dwt_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// 获取当前 CPU 周期计数
static inline uint32_t dwt_get_cycles(void) {
    return DWT->CYCCNT;
}

// 获取 CPU 主频（单位：Hz）
uint32_t get_cpu_freq_hz(void) {
    return SystemCoreClock;  // 通常是 400MHz（具体由配置决定）
}


int test_memory_region(uint32_t start_addr, uint32_t size, const char* name) {
    volatile uint32_t* p = (volatile uint32_t*)start_addr;
    uint32_t words = size / 4;
    uint32_t i;
    
    // Use a sampling step to test only portions of memory
    uint32_t step = (words > 1000) ? (words / 1000) : 1;  // Sample about 1000 locations
    
    printf("Checking %s (%lu KB) ", name, size / 1024);

    // Write pattern to entire memory
    for (i = 0; i < words; i++) {
        p[i] = 0xAA55BB66;
    }

    // Verify sampled locations
    for (i = 0; i < words; i += step) {
        if (p[i] != 0xAA55BB66) {
            printf("FAILED at 0x%08X\r\n", (uint32_t)&p[i]);
            return -1;
        }
    }

    printf("=> OK \n\r");
    return 0;
}





void memory_test_all() 
{
    uint32_t cpu_freq = get_cpu_freq_hz();
    printf("CPU Frequency: %lu MHz\r\n", cpu_freq / 1000000);

    printf("  I-Cache: %s\r\n", (SCB->CCR & SCB_CCR_IC_Msk) ? "Enabled" : "Disabled");
    printf("  D-Cache: %s\r\n", (SCB->CCR & SCB_CCR_DC_Msk) ? "Enabled" : "Disabled");
    printf("\r\n"); 

    printf("--- Memory Tests ---\r\n");
    test_memory_region(ITCMRAM_START, ITCMRAM_SIZE, "ITCMRAM");
    test_memory_region(DTCMRAM_START, DTCMRAM_SIZE, "DTCMRAM");
    test_memory_region(RAM_D1_START, RAM_D1_SIZE, "RAM_D1");
    test_memory_region(RAM_D2_START, RAM_D2_SIZE, "RAM_D2");
    test_memory_region(RAM_D3_START, RAM_D3_SIZE, "RAM_D3");
    test_memory_region(SDRAM_START_ADDR, SDRAM_SIZE_BYTES, "SDRAM");
}





void print_boot_header() {
    printf("\r\n\r\n");
    printf("Bootloader for STM32H743\r\n");
    printf("--------------------------------------\r\n");
    printf("GCC %d.%d.%d (%s)\r\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __VERSION__);
    #if defined(__x86_64__) || defined(__aarch64__)
        printf("Architecture: 64-bit target\r\n");
    #else
        printf("Architecture: 32-bit target\r\n");
    #endif
    printf("Build Date: %s %s\r\n", __DATE__, __TIME__); // 增加编译时间
}

void jmp_to_start(uint32_t addr);

void load_kernel() 
{
    print_boot_header();      
    dwt_init();
    memory_test_all();   
    printf("System total memory: ~%lu KB\r\n", 
       (DTCMRAM_SIZE + RAM_D1_SIZE + RAM_D2_SIZE + ITCMRAM_SIZE + RAM_D3_SIZE + SDRAM_SIZE_BYTES) / 1024);
      
    jmp_to_start(KERNEL_START_ADDRESS);
}

void jmp_to_start(uint32_t addr) {
    uint32_t msp_value = *(volatile uint32_t*)addr;
    uint32_t start_addr = *(volatile uint32_t*)(addr + 4);
    printf("Booting kernel from 0x%08X...\r\n", addr);
    printf("  Entry Point: 0x%08X\r\n", start_addr);
    printf("  Initial MSP: 0x%08X\r\n", msp_value);
       printf("Relocating to kernel...\r\n"); 

    __disable_irq();
    SCB->VTOR = addr;
    __DSB();
    __ISB();
    __set_MSP(msp_value);     
    int (*kernel_main)(void*) = (int (*)(void*))(start_addr);
    kernel_main(_send_byte); 

    printf("\r\n!!! ERROR: Kernel did not return. Check application code. !!!\r\n");
    while (1); 
}