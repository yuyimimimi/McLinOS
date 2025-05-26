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
#include <linux/interrupt.h>
#include <linux/irqflags.h>

unsigned int major = 0;

#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define USART1_BASE     0x40011000
#define NVIC_BASE       0xE000E100

#define RCC             ((volatile uint32_t*)RCC_BASE)
#define GPIOA           ((GPIO_TypeDef*)GPIOA_BASE)
#define USART1          ((USART_TypeDef*)USART1_BASE)

#define NVIC_ISER0      (*(volatile uint32_t*)(NVIC_BASE + 0x00))
#define NVIC_ISER1      (*(volatile uint32_t*)(NVIC_BASE + 0x04))
#define NVIC_ICPR1      (*(volatile uint32_t*)(NVIC_BASE + 0x284))
#define NVIC_IPR_BASE   ((volatile uint8_t*)(NVIC_BASE + 0x300))

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;



irqreturn_t  USART1_IRQHandler(int i,void *argv);

static u32 irq_num;
void uart1_device_init_function_from_dtb(void)
{
    irq_num = 37;  // 硬编码 USART1 IRQ for simplicity
    u32 baudrate = 115200;  // 默认波特率

    // 1. 使能 GPIOA 和 USART1 时钟
    RCC[0x30 / 4] |= (1 << 0);   // GPIOAEN
    RCC[0x44 / 4] |= (1 << 4);   // USART1EN

    // 2. 配置 PA9(TX) PA10(RX) 复用功能为 AF7
    GPIOA->MODER &= ~(0xF << 18);
    GPIOA->MODER |=  (0xA << 18);       // MODER9 = AF, MODER10 = AF
    GPIOA->AFR[1] &= ~(0xFF << 4);
    GPIOA->AFR[1] |=  (0x77 << 4);      // AFR9, AFR10 = AF7
    GPIOA->OSPEEDR |= (0xF << 18);      // High speed
    GPIOA->PUPDR &= ~(0xF << 18);
    GPIOA->PUPDR |=  (0x5 << 18);       // Pull-up for RX, none for TX
    GPIOA->OTYPER &= ~(0x3 << 9);       // Push-pull

    // 3. USART 配置
    USART1->CR1 = 0;  // 清除所有控制位
    USART1->BRR = (84000000 + baudrate / 2) / baudrate;  // 波特率设置
    USART1->CR1 |= (1 << 2) | (1 << 3);  // RE + TE
    USART1->CR1 |= (1 << 5);             // RXNEIE: 接收中断使能
    USART1->CR1 |= (1 << 13);            // UE: USART使能

    // 4. 配置 NVIC：使能中断、设置优先级、清除 pending
    if (irq_num < 32) {
        NVIC_ISER0 |= (1 << irq_num);
    } else {
        NVIC_ISER1 |= (1 << (irq_num - 32));
        NVIC_ICPR1 |= (1 << (irq_num - 32));  // 清 pending
    }

    NVIC_IPR_BASE[irq_num] = (3 << 4);  // 设置中断优先级（高 4 位有效）
}

void base_out_opt_device_init(void){
    uart1_device_init_function_from_dtb();
}

static void USART_1_SendByte(uint8_t byte){
    while(!(USART1->SR & (1 << 7)));
    USART1->DR = byte;
}

#define RX_BUFFER_SIZE 1024
static char usrat1_rx_buffer[RX_BUFFER_SIZE];
static uint16_t usrat1_rx_index_rear = 0;  // 接收缓冲区尾指针
static uint16_t usrat1_rx_index_front = 0; // 接收缓冲区头指针
irqreturn_t  USART1_IRQHandler(int i,void *argv){
    if(USART1->SR & (1 << 5)) {
        uint8_t data = USART1->DR;
        USART1->SR &= ~(1 << 5);
        //USART_1_SendByte(data);
        uint16_t next_rear = (usrat1_rx_index_front + 1) % RX_BUFFER_SIZE;     //获取头指针下一次所在点
        if(next_rear == usrat1_rx_index_rear)                                  //如果存储下一个字节的位点为尾指针说明缓冲区已满
            usrat1_rx_index_rear = (usrat1_rx_index_rear + 1) % RX_BUFFER_SIZE;//尾指针前移一位丢弃1bit数据
        usrat1_rx_buffer[usrat1_rx_index_front] = data;                        //将数据添加到缓冲区
        usrat1_rx_index_front = next_rear;  //更新头指针处

    }
    return IRQ_HANDLED;
}

static int get_rx_buffer_state(){
    if(usrat1_rx_index_rear != usrat1_rx_index_front){ //如果缓冲区非空
        return 1;
    }
    return 0;
}
static char get_rx_buffer_data()                                 //从尾指针获取缓冲区数据
{
    char byte = usrat1_rx_buffer[usrat1_rx_index_rear];
    usrat1_rx_index_rear = (usrat1_rx_index_rear + 1) % RX_BUFFER_SIZE;
    return byte;
}

static int Usart1_read(char *data){
    local_irq_disable();
    if(get_rx_buffer_state() == 1){
        data[0] = get_rx_buffer_data();
        local_irq_enable();
        return 0;
    }
    local_irq_enable();
    return -1;
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

static ssize_t tty_read(struct file *file, const char __user *buserdata, size_t size, loff_t *offset){    
    size_t i;
    for(i =0;i<size;i++){
       if(Usart1_read(&buserdata[i]) < 0)
       {
         break;        
       }
    }
    return i;
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
    .release = tty_close,
    .read   = tty_read
};


int __init main_tty_dev_init(void)
{
    request_irq(irq_num,USART1_IRQHandler,NULL,"usart1",NULL);
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



