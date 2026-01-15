#include <linux/kernel.h>
#include <hardware/systick.h>


#define M0PLUS_SYST_CSR_ENABLE_BITS   _u(0x00000001)
#define M0PLUS_SYST_CSR_TICKINT_BITS   _u(0x00000002)
#define M0PLUS_SYST_CSR_CLKSOURCE_BITS   _u(0x00000004)

void SysTick_init(void)
{
    systick_hw->rvr = (150*1000*1000 / 1000 - 1);
    systick_hw->cvr = 0;
    systick_hw->csr = M0PLUS_SYST_CSR_ENABLE_BITS | 
                      M0PLUS_SYST_CSR_TICKINT_BITS | 
                      M0PLUS_SYST_CSR_CLKSOURCE_BITS;
}

extern void sys_tick_hander(void);
void SysTick_Handler(void){
    sys_tick_hander();
} 
