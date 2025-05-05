#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h> 
#include <linux/stdarg.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/devfs.h>
#include <linux/init.h>
#include <linux/sprintf.h>
#include <linux/stdarg.h>
#include <linux/device.h>
#include <linux/of.h>
#include <generated/autoconf.h>


unsigned int major = 0;

#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define USART1_BASE     0x40011000
#define NVIC_BASE       0xE000E100

typedef struct {
    volatile uint32_t MODER;    // 模式寄存器
    volatile uint32_t OTYPER;   // 输出类型寄存器
    volatile uint32_t OSPEEDR;  // 输出速度寄存器
    volatile uint32_t PUPDR;    // 上下拉寄存器
    volatile uint32_t IDR;      // 输入数据寄存器
    volatile uint32_t ODR;      // 输出数据寄存器
    volatile uint32_t BSRR;     // 位设置/清除寄存器
    volatile uint32_t LCKR;     // 配置锁定寄存器
    volatile uint32_t AFR[2];   // 复用功能寄存器
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t SR;       // 状态寄存器
    volatile uint32_t DR;       // 数据寄存器
    volatile uint32_t BRR;      // 波特率寄存器
    volatile uint32_t CR1;      // 控制寄存器1
    volatile uint32_t CR2;      // 控制寄存器2
    volatile uint32_t CR3;      // 控制寄存器3
    volatile uint32_t GTPR;     // 保护时间和预分频寄存器
} USART_TypeDef;


#define RCC     ((volatile uint32_t*)RCC_BASE)
#define GPIOA   ((GPIO_TypeDef*)GPIOA_BASE)
#define USART1  ((USART_TypeDef*)USART1_BASE)
#define NVIC_ISER0  (*(volatile uint32_t*)(NVIC_BASE + 0x00))
#define NVIC_IPR9   (*(volatile uint32_t*)(NVIC_BASE + 0x424))

static void uart1_device_init_function_from_dtb(void)
{

    struct device_node *uart_np;
    u32 reg[2];
    u32 irq_num = 0;
    u32 baudrate = CONFIG_UART1_SPEED; 
    const char *path = "/soc/serial@40011000";

    uart_np = of_find_compatible_node(NULL, NULL, "st,stm32-uart");
    if (!uart_np) {
        uart_np = of_find_node_by_path(path);
        if (!uart_np) {
            return;
        }
    }

    if (of_property_read_u32_array(uart_np, "reg", reg, 2) != 0)
    of_property_read_u32_array(uart_np, "interrupts", &irq_num, 1);
    of_property_read_u32(uart_np, "current-speed", &baudrate);

    USART_TypeDef *uart = (USART_TypeDef *)reg[0];

    RCC[0x30/4] |= (1 << 0);          
    GPIOA->MODER &= ~(0xF << 18);     
    GPIOA->MODER |= (0xA << 18);      
    GPIOA->AFR[1] &= ~(0xFF << 4);
    GPIOA->AFR[1] |= (0x77 << 4);    
    GPIOA->OSPEEDR |= (0xF << 18);
    GPIOA->PUPDR &= ~(0xF << 18);
    GPIOA->PUPDR |= (0x5 << 18);
    GPIOA->OTYPER &= ~(0x3 << 9);

    RCC[0x44/4] |= (1 << 4);

    uart->CR1 = 0;
    uart->BRR = (84000000 + baudrate/2) / baudrate;
    uart->CR1 |= (1 << 3) | (1 << 2);
    uart->CR1 |=  (1 << 13) ;
    uart->CR1 |= (1 << 5) ;

    if (irq_num < 32)
        NVIC_ISER0 |= (1 << (irq_num & 0x1F));

    if (irq_num >= 36 && irq_num <= 39) {
        NVIC_IPR9 |= (3 << ((irq_num - 36) * 8 + 6)); 
    }
}

void base_out_opt_device_init(void){
    uart1_device_init_function_from_dtb();
}

static void USART_1_SendByte(uint8_t byte)
{
    while(!(USART1->SR & (1 << 7)));
    USART1->DR = byte;
}

void early_printk(const char *fmt, ...){
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    for(int i = 0; buf[i]!= '\0'; i++){
        USART_1_SendByte(buf[i]);
        if(buf[i] == '\n')
        USART_1_SendByte('\r');
    }
}  
 

static int tty_open(struct inode *node, struct file *file){
    return 0;
}

static ssize_t tty_write(struct file *file, const char __user *buserdata, size_t size, loff_t *offset){    
    for(int i = 0;i< size ;i++){
        USART_1_SendByte(buserdata[i]);     
        if(buserdata[i] == '\n'){
            USART_1_SendByte('\r');
        }
    }
    return size;
}

static int tty_close(struct inode *node, struct file *file){
    return 0;
}

static struct file_operations tty_fop = {
    .open = tty_open,
    .write = tty_write,
    .release = tty_close
};


int __init main_tty_dev_init(void)
{

    major = register_chrdev(major,"ttyS0",&tty_fop);
    if(major < 0){
        pr_info("can not get major:(%d)\n",major);
    }
    pr_info("register ttyS0 (%d)\n",major);

    struct class * tty_dev = class_create(THIS_MODULE,"ttyS0");
    if(IS_ERR(tty_dev)){
        pr_info("can not create tty class\n");
    }
    struct device * dev = device_create(tty_dev,NULL,major,NULL,"ttyS%d",0);
    if(IS_ERR(tty_dev)){
        pr_info("can not create tty device\n");
    }
}






