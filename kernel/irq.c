#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <generated/autoconf.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/irqflags.h>
#include <linux/spinlock.h>
#include <linux/init.h>

struct irqaction* IRQ_list[CONFIG_INTERRUPT_NUMBERS];
spinlock_t ire_list_lock;


static int Run_IRQ_Fn(struct irqaction* IRQ);
void C_IRQ_Dispatcher(int irq_number)
{
    if(irq_number >= CONFIG_INTERRUPT_NUMBERS)
    return;

    if(Run_IRQ_Fn(IRQ_list[irq_number]) == 0){
        printk(KERN_WARNING "unregister interrpute:(%d)",irq_number);
    }
}


static int Run_IRQ_Fn(struct irqaction* IRQ)
{
    int j = 0;
    struct irqaction* i = IRQ;
    while (1)
    {
        if(i == NULL)
        break;
        i->flags = i->handler(i->irq,i->dev_id); 
        i = i->next;
        j++;
    }
    return j;
}


static struct irqaction* new_irq_create(char *name)
{
    struct irqaction* new = kmalloc(sizeof(struct irqaction),GFP_KERNEL);
    if(new == NULL){
        return  PTR_ERR(-ENOMEM);
    }
    char *new_name = kmalloc(strlen(name) + 1,GFP_KERNEL);
    if(new_name == NULL){
        kfree(new);
        return PTR_ERR(-ENOMEM);
    }
    memset(new,0,sizeof(struct irqaction));
    memcpy(new_name,name,strlen(name));
    new->name = new_name;
    
    printk(KERN_INFO "new_irq_create: Created IRQ action '%s' at %p\n", new_name, new);
    return new;
}

static void add_irq_to_table(struct irqaction* new)
{
    spin_lock(&ire_list_lock);
    local_irq_disable();
    uint32_t irq_number = new->irq;
    if(IRQ_list[irq_number] == NULL){
        IRQ_list[irq_number] = new;
        goto out;
    }

    struct irqaction* i = IRQ_list[irq_number];
    while (1)
    {
        if(i->next == NULL){
            new->secondary = i;
            i->next = new;
            goto out;
        }
        i = i->next;
    }

    out:
    local_irq_enable();
    spin_unlock(&ire_list_lock);
}


void remove_irq(struct irqaction* remove)
{
    spin_lock(&ire_list_lock);
    local_irq_disable();
    if(IRQ_list[remove->irq] == NULL){
        goto out;
    }
    if(IRQ_list[remove->irq] == remove){
        IRQ_list[remove->irq] = NULL;
        goto out;
    }

    if(remove->secondary)
    remove->secondary->next = remove->next;
    if(remove->next)
    remove->next->secondary = remove->secondary;

    out:
    local_irq_enable();
    spin_unlock(&ire_list_lock);
}


int request_irq(unsigned int irq,
                irq_handler_t handler,
                unsigned long flags,
                const char *name,
                void *dev)
{
    struct irqaction* new = new_irq_create(name);
    if(IS_ERR(new)){
        return PTR_ERR(new);
    }
    new->irq = irq;
    new->handler = handler;
    new->flags = flags;
    new->dev_id = dev;
    add_irq_to_table(new);
    return 0;
}


static int IRQ_List_Init()
{
    for(int i =0;i<CONFIG_INTERRUPT_NUMBERS;i++){
        IRQ_list[i] = NULL;
    }
    return 0;
}
void IRQ_int_router(){
    spin_lock_init(&ire_list_lock);
    IRQ_List_Init();
}

// irq：中断号
// handler：你的中断处理函数指针，形如 irqreturn_t handler(int irq, void *dev_id)
// flags：中断触发方式（如 IRQF_SHARED）
// name：中断名字（用于调试）
// dev：设备私有数据指针