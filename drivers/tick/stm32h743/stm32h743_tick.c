#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <generated/autoconf.h>




// ====================== SCB 定义 (适配 Cortex-M7) ======================
#define SCB_BASE            (0xE000ED00UL)
#define SCB                 ((SCB_Type *) SCB_BASE)

typedef struct {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint8_t  SHP[12];  // 系统异常优先级寄存器 (用于设置SysTick优先级)
    volatile uint32_t SHCSR;
    volatile uint32_t CFSR;
    volatile uint32_t HFSR;
    volatile uint32_t DFSR;
    volatile uint32_t MMFAR;
    volatile uint32_t BFAR;
    volatile uint32_t AFSR;
    volatile uint32_t ID_PFR[2];
    volatile uint32_t ID_DFR;
    volatile uint32_t ID_AFR;
    volatile uint32_t ID_MFR[4];
    volatile uint32_t ID_ISAR[5];
} SCB_Type;

// ====================== SysTick 定义 ======================
#define SYSTICK_BASE        (0xE000E010UL)
#define SysTick             ((SysTick_Type *) SYSTICK_BASE)

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_Type;

#define SYSTICK_CTRL_ENABLE_Pos      0
#define SYSTICK_CTRL_TICKINT_Pos     1
#define SYSTICK_CTRL_CLKSOURCE_Pos   2
#define SYSTICK_CTRL_COUNTFLAG_Pos   16

#define SYSTICK_CTRL_ENABLE_Msk      (1UL << SYSTICK_CTRL_ENABLE_Pos)
#define SYSTICK_CTRL_TICKINT_Msk     (1UL << SYSTICK_CTRL_TICKINT_Pos)
#define SYSTICK_CTRL_CLKSOURCE_Msk   (1UL << SYSTICK_CTRL_CLKSOURCE_Pos)
#define SYSTICK_CTRL_COUNTFLAG_Msk   (1UL << SYSTICK_CTRL_COUNTFLAG_Pos)

// ====================== 用户配置宏 ======================
// 默认 SystemCoreClock 使用 H743 的常见频率 (自己可改)
uint32_t SystemCoreClock = 400000000;  // 400MHz (例如使用 PLL1 生成的 SYSCLK)

// 可配置 tick 频率，例如 1ms=1000Hz
#define CONFIG_SYSTEM_TICK_FREQUENCY 1000

volatile uint32_t sys_tick = 0;

// ====================== SysTick 初始化函数 ======================
void SysTick_init(void)
{
    // 1. 禁用SysTick
    SysTick->CTRL = 0;

    // 2. 检查系统时钟是否有效
    if (SystemCoreClock == 0)
        while (1); // 死循环

    // 3. 计算重装载值
    uint32_t load = (SystemCoreClock / CONFIG_SYSTEM_TICK_FREQUENCY) - 1;
    if (load > 0xFFFFFF)
        load = 0xFFFFFF;

    SysTick->LOAD = load;

    // 4. 设置 SysTick 优先级（异常号 15 → SHP[11]）
    SCB->SHP[11] = 0;  // 最高优先级

    // 5. 清除当前值
    SysTick->VAL = 0;

    // 6. 启用 SysTick (使用核心时钟)
    SysTick->CTRL = SYSTICK_CTRL_CLKSOURCE_Msk |
                    SYSTICK_CTRL_TICKINT_Msk |
                    SYSTICK_CTRL_ENABLE_Msk;
}


extern void sys_tick_hander(void);

void SysTick_Handler(void){
    sys_tick_hander();
}

