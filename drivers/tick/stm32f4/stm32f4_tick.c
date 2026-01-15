#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <generated/autoconf.h>




// 系统控制块 (SCB) 寄存器定义
#define SCB_BASE            (0xE000ED00UL)
#define SCB                 ((SCB_Type *) SCB_BASE)
 
typedef struct {
  volatile uint32_t CPUID;                  // CPUID基址寄存器
  volatile uint32_t ICSR;                   // 中断控制及状态寄存器
  volatile uint32_t VTOR;                   // 向量表偏移寄存器
  volatile uint32_t AIRCR;                  // 应用中断及复位控制寄存器
  volatile uint32_t SCR;                    // 系统控制寄存器
  volatile uint32_t CCR;                    // 配置控制寄存器
  volatile uint8_t  SHP[12];                // 系统异常优先级寄存器 (4-15)
  volatile uint32_t SHCSR;                  // 系统异常状态和控制寄存器
  volatile uint32_t CFSR;                   // 可配置错误状态寄存器
  volatile uint32_t HFSR;                   // 硬件错误状态寄存器
  volatile uint32_t DFSR;                   // 调试错误状态寄存器
  volatile uint32_t MMFAR;                  // 存储器管理错误地址寄存器
  volatile uint32_t BFAR;                   // 总线错误地址寄存器
  volatile uint32_t AFSR;                   // 辅助错误状态寄存器
  volatile uint32_t ID_PFR[2];              // 处理器特性寄存器
  volatile uint32_t ID_DFR;                 // 调试特性寄存器
  volatile uint32_t ID_AFR;                 // 辅助特性寄存器
  volatile uint32_t ID_MFR[4];              // 存储器模型特性寄存器
  volatile uint32_t ID_ISAR[5];             // 指令集属性寄存器
} SCB_Type;

// SysTick 寄存器定义
#define SYSTICK_BASE        (0xE000E010UL)
#define SysTick            ((SysTick_Type *) SYSTICK_BASE)

typedef struct {
  volatile uint32_t CTRL;                   // 控制和状态寄存器
  volatile uint32_t LOAD;                   // 重装载值寄存器
  volatile uint32_t VAL;                    // 当前值寄存器
  volatile uint32_t CALIB;                  // 校准值寄存器
} SysTick_Type;


#define SYSTICK_CTRL_ENABLE_Pos             0
#define SYSTICK_CTRL_TICKINT_Pos            1
#define SYSTICK_CTRL_CLKSOURCE_Pos          2
#define SYSTICK_CTRL_COUNTFLAG_Pos          16

#define SYSTICK_CTRL_ENABLE_Msk            (1UL << SYSTICK_CTRL_ENABLE_Pos)
#define SYSTICK_CTRL_TICKINT_Msk           (1UL << SYSTICK_CTRL_TICKINT_Pos)
#define SYSTICK_CTRL_CLKSOURCE_Msk         (1UL << SYSTICK_CTRL_CLKSOURCE_Pos)
#define SYSTICK_CTRL_COUNTFLAG_Msk         (1UL << SYSTICK_CTRL_COUNTFLAG_Pos)


uint32_t SystemCoreClock = 160000000;

volatile uint32_t sys_tick = 0;

void SysTick_init(void)
{
    // 1. 禁用SysTick
    SysTick->CTRL = 0;
    
    // 2. 检查系统时钟是否有效
    if (SystemCoreClock == 0) {
        while (1); // 系统时钟无效，死循环
    }
    
    // 3. 计算重装载值
    uint32_t load = (SystemCoreClock / CONFIG_SYSTEM_TICK_FREQUENCY) - 1;
    if (load > 0xFFFFFF) {  
        load = 0xFFFFFF; // 最大值限制
    }
    SysTick->LOAD = load;
    
    // 4. 设置优先级 (SysTick是内核异常，优先级通过SCB->SHP设置)
    // Cortex-M允许设置8位优先级，STM32通常只使用高4位
    // 设置优先级为0 (最高优先级)
    SCB->SHP[11] = 0;  // SysTick是异常#15，SHP[11]对应异常12-15
    
    // 5. 清除当前值
    SysTick->VAL = 0;
    
    // 6. 配置并启用SysTick
    // 使用处理器时钟 (CLKSOURCE=1)
    // 启用中断 (TICKINT=1)
    // 启用计数器 (ENABLE=1)
    SysTick->CTRL = SYSTICK_CTRL_CLKSOURCE_Msk | 
                    SYSTICK_CTRL_TICKINT_Msk | 
                    SYSTICK_CTRL_ENABLE_Msk;
}


void SysTick_Handler(void)
{
    // 用户定义的tick处理函数
    extern void sys_tick_hander(void);
    sys_tick_hander();
}

