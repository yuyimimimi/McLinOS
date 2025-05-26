#include <linux/types.h>
#include "core_cm4.h"

uint32_t SystemCoreClock = 160000000;

#define RCC_BASE        0x40023800
#define FLASH_BASE      0x40023C00

#define RCC             ((RCC_TypeDef *)RCC_BASE)
#define FLASH           ((FLASH_TypeDef *)FLASH_BASE)

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    uint32_t RESERVED0;
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    uint32_t RESERVED2;
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    uint32_t RESERVED3[2];
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    volatile uint32_t AHB3LPENR;
    uint32_t RESERVED4;
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    uint32_t RESERVED5[2];
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    uint32_t RESERVED6[2];
    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2SCFGR;
    volatile uint32_t PLLSAICFGR;
    volatile uint32_t DCKCFGR;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t OPTCR;
} FLASH_TypeDef;





void SystemClock_Config(void)
{
    // 1. 使能 HSI
    RCC->CR |= (1 << 0); // HSION
    while (!(RCC->CR & (1 << 1))); // HSIRDY

    // 2. 配置 Flash 等待周期，必须在切换系统频率前设置好
    FLASH->ACR |= (5 << 0); // 5 个 wait state，168 MHz 推荐值
    FLASH->ACR |= (1 << 8); // DCache enable
    FLASH->ACR |= (1 << 9); // ICache enable
    FLASH->ACR |= (1 << 10); // Prefetch enable

    // 3. 关闭 PLL
    RCC->CR &= ~(1 << 24); // PLL OFF
    while (RCC->CR & (1 << 25)); // 等待 PLLRDY 置 0

    // 4. 配置 PLL
    /*
        PLLCFGR = 
        [5:0]   PLLM = 16
        [14:6]  PLLN = 336
        [17:16] PLLP = 2 (00)
        [22]    PLLSRC = 0 (HSI)
        [27:24] PLLQ = 7 (USB OTG 48MHz)
    */
    RCC->PLLCFGR = (16 << 0) | (336 << 6) | (0 << 16) | (0 << 22) | (7 << 24);

    // 5. 打开 PLL
    RCC->CR |= (1 << 24); // PLLON
    while (!(RCC->CR & (1 << 25))); // 等待 PLLRDY

    // 6. 配置 AHB, APB 分频
    /*
        HPRE[7:4]  = 0b0000 (不分频)
        PPRE1[12:10]= 0b101 (168MHz / 4 = 42MHz)
        PPRE2[15:13]= 0b100 (168MHz / 2 = 84MHz)
    */
    RCC->CFGR &= ~(0xF << 4);    // AHB prescaler 清零
    RCC->CFGR &= ~(0x7 << 10);   // APB1 prescaler 清零
    RCC->CFGR |= (0x5 << 10);    // APB1 = /4
    RCC->CFGR &= ~(0x7 << 13);   // APB2 prescaler 清零
    RCC->CFGR |= (0x4 << 13);    // APB2 = /2

    // 7. 切换系统时钟到 PLL
    RCC->CFGR &= ~(0x3 << 0); // 清除 SW
    RCC->CFGR |= (0x2 << 0);  // 选择 PLL 作为系统时钟

    while ((RCC->CFGR & (0x3 << 2)) != (0x2 << 2)); // 等待切换完成



}



