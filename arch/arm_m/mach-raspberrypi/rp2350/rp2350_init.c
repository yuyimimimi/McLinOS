#include <linux/kernel.h>
#include <generated/autoconf.h>
#include <linux/irqflags.h>
#include <linux/string.h>
#include <asm/barrier.h>
#include <hardware/structs/scb.h>
#include <linux/interrupt.h>





void __hardful_spinlock_init(volatile int8_t *lock);

#define     __IOM    volatile         
#define     __OM     volatile   

typedef struct
{
  __IOM uint32_t ISER[16U];              /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register */
        uint32_t RESERVED0[16U];
  __IOM uint32_t ICER[16U];              /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
        uint32_t RSERVED1[16U];
  __IOM uint32_t ISPR[16U];              /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register */
        uint32_t RESERVED2[16U];
  __IOM uint32_t ICPR[16U];              /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
        uint32_t RESERVED3[16U];
  __IOM uint32_t IABR[16U];              /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
        uint32_t RESERVED4[16U];
  __IOM uint32_t ITNS[16U];              /*!< Offset: 0x280 (R/W)  Interrupt Non-Secure State Register */
        uint32_t RESERVED5[16U];
  __IOM uint8_t  IPR[496U];              /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
        uint32_t RESERVED6[580U];
  __OM  uint32_t STIR;                   /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
}  NVIC_Type;



#define SCS_BASE            (0xE000E000UL)  
#define NVIC_BASE           (SCS_BASE +  0x0100UL)   
// #define NVIC_BASE       (0xE000E100)

#define NVIC                ((NVIC_Type      *)     NVIC_BASE   )

#define NVIC_SETENA     ((volatile uint32_t *)(NVIC_BASE + 0x000))
#define NVIC_CLRENA     ((volatile uint32_t *)(NVIC_BASE + 0x080))

void irq_set_enabled(uint32_t num, bool enabled) {
    uint32_t reg_index = num >> 5;            // num / 32
    uint32_t bit_mask = 1u << (num & 0x1F);   // 1 << (num % 32)

    if (enabled) {
        NVIC_SETENA[reg_index] = bit_mask;    // 设置中断使能位
    } else {
        NVIC_CLRENA[reg_index] = bit_mask;    // 清除中断使能位
    }
}
bool irq_is_enabled(uint32_t num) {
    uint32_t reg_index = num >> 5;
    uint32_t bit_mask = 1u << (num & 0x1F);

    uint32_t reg_value = NVIC_SETENA[reg_index];
    return (reg_value & bit_mask) != 0;
}


void clean_interrupt_flag(uint32_t irq_num) 
{
    int IRQn = irq_num;
    if ((int)(IRQn) > 1){
        pr_info("clean flas:%d\n",IRQn);
        irq_set_enabled(IRQn,0);
        NVIC->ICPR[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));
    }
}


extern void SysTick_Handler(void);

irqreturn_t  CPU1_TickHandler(int i,void *argv)
{
    SysTick_Handler();
   //pr_info("cpu%d running\n",__get_task_using_cpu_core());
    return IRQ_HANDLED;
}



void arch_init()
{
    uint32_t msp, psp, vtor, control;
    __asm volatile (
        "mrs %0, msp\n"
        "mrs %1, psp\n"
        "mrs %2, control\n"
        : "=r"(msp), "=r"(psp), "=r"(control)
    );
    vtor = scb_hw->vtor;
    pr_info("CPU: cortex_m33\n");
    pr_info("MSP: 0x%08x PSP: 0x%08x VTOR: 0x%08x CONTROL: 0x%08x Current PC: 0x%08x\n", msp, psp, vtor, control,__builtin_return_address(0));          // 主栈指针
    uint32_t *vector_table = (uint32_t *)CONFIG_LD_FLASH_START_ADDRESS;
    pr_info("Vector Table[0] (MSP): 0x%08x Vector Table[1] (Reset_Handler): 0x%08x\n", vector_table[0], vector_table[1]);
    barrier();
    
    bool irq_enabled = irq_is_enabled(0x19);
    irq_set_enabled(0x19, false);

    memcpy(vtor,CONFIG_LD_FLASH_START_ADDRESS,(68)*4);

    irq_set_enabled(0x19, irq_enabled);
   
    pr_info("New VTOR: 0x%08x r2350 arch init done\n", scb_hw->vtor);

    request_irq(1,CPU1_TickHandler,NULL,"CPU1_Tick",NULL);    

}
