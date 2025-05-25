#ifndef  __IRQFLAG_H_
#define  __IRQFLAG_H_

extern void __enable_irq();
extern void __disble_irq();

static void local_irq_disable(){
    __disble_irq();
}

static void local_irq_enable(){
    __enable_irq();
}





#endif // ! 