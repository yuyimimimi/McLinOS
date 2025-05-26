extern void C_IRQ_Dispatcher(int irq_number);
void IRQ_Generic_Handler(void)
{
     int irq_number;
    __asm volatile (
        "MRS %0, IPSR\n\t"   // 读 IPSR 到 irq_number
        "SUB %0, %0, #16\n\t"
        : "=r" (irq_number)
    );
    C_IRQ_Dispatcher(irq_number);
}


void __enable_irq(){
    __asm volatile ("cpsie i");
}

void __disble_irq(){
    __asm volatile ("cpsid i");
}
