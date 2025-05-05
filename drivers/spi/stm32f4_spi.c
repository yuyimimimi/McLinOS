#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/gpio_subsystem.h>

// 基础地址
#define PERIPH_BASE           ((unsigned int)0x40000000)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000)
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000)

// GPIOA
#define GPIOA_BASE            (AHB1PERIPH_BASE + 0x0000)
#define GPIOA_MODER           (*(volatile unsigned int *)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER          (*(volatile unsigned int *)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR         (*(volatile unsigned int *)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR           (*(volatile unsigned int *)(GPIOA_BASE + 0x0C))
#define GPIOA_AFRL            (*(volatile unsigned int *)(GPIOA_BASE + 0x20))  // 低8位引脚AFR

// RCC
#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800)
#define RCC_AHB1ENR           (*(volatile unsigned int *)(RCC_BASE + 0x30))
#define RCC_APB2ENR           (*(volatile unsigned int *)(RCC_BASE + 0x44))
#define RCC_APB2RSTR          (*(volatile unsigned int *)(RCC_BASE + 0x24))

// SPI1
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000)
#define SPI1_CR1              (*(volatile unsigned int *)(SPI1_BASE + 0x00))
#define SPI1_CR2              (*(volatile unsigned int *)(SPI1_BASE + 0x04))
#define SPI1_SR               (*(volatile unsigned int *)(SPI1_BASE + 0x08))
#define SPI1_DR               (*(volatile unsigned int *)(SPI1_BASE + 0x0C))

// 位定义
#define RCC_AHB1ENR_GPIOAEN   (1 << 0)
#define RCC_APB2ENR_SPI1EN    (1 << 12)
#define RCC_APB2RSTR_SPI1RST  (1 << 12)

#define SPI_CR1_SPE           (1 << 6)
#define SPI_CR1_MSTR          (1 << 2)
#define SPI_CR1_CPOL          (1 << 1)
#define SPI_CR1_CPHA          (1 << 0)
#define SPI_CR1_DFF           (1 << 11)
#define SPI_CR1_LSBFIRST      (1 << 7)
#define SPI_CR1_SSM           (1 << 9)
#define SPI_CR1_SSI           (1 << 8)

#define SPI_SR_TXE            (1 << 1)
#define SPI_SR_RXNE           (1 << 0)



static void SPI1_Init(void)
{
    // 1. 使能GPIOA时钟
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // 2. 配置PA5, PA6, PA7为复用模式
    GPIOA_MODER &= ~((3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOA_MODER |=  ((2 << (5 * 2)) | (2 << (6 * 2)) | (2 << (7 * 2)));

    // 3. 推挽输出
    GPIOA_OTYPER &= ~((1 << 5) | (1 << 6) | (1 << 7));

    // 4. 高速
    GPIOA_OSPEEDR |= ((3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));

    // 5. 无上下拉
    GPIOA_PUPDR &= ~((3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));

    // 6. 配置PA5, PA6, PA7 复用为AF5 (SPI1)
    GPIOA_AFRL &= ~((0xF << (5 * 4)) | (0xF << (6 * 4)) | (0xF << (7 * 4)));
    GPIOA_AFRL |=  ((5 << (5 * 4)) | (5 << (6 * 4)) | (5 << (7 * 4)));

    // 7. 使能SPI1时钟
    RCC_APB2ENR |= RCC_APB2ENR_SPI1EN;

    // 8. SPI1复位
    RCC_APB2RSTR |= RCC_APB2RSTR_SPI1RST;
    RCC_APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;

    // 9. SPI1参数配置
    SPI1_CR1 = 0;
    SPI1_CR1 |= SPI_CR1_MSTR;      // 主模式
    SPI1_CR1 |= SPI_CR1_CPOL;      // 空闲高电平
    SPI1_CR1 |= SPI_CR1_CPHA;      // 第二个边沿采样
    SPI1_CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;  // 软件管理NSS
    SPI1_CR1 |= (0 << 3);          // 默认分频：fPCLK/2

    // 10. 使能SPI1
    SPI1_CR1 |= SPI_CR1_SPE;
}

static void SPI1_SetSpeed(uint8_t prescaler)
{
    SPI1_CR1 &= ~(7 << 3);  // 清除BR[2:0]位
    SPI1_CR1 |= (prescaler << 3); // 设置新的分频值
}



unsigned short get_spi_speed(unsigned int speed)
{
    unsigned int pclk2 = 168000000 / 2; // 默认核心频率是168MHz
    if(speed >= pclk2/2 )   return 0;
    if(speed >= pclk2/4 )   return 1;
    if(speed >= pclk2/8 )   return 2;
    if(speed >= pclk2/16 )  return 3;
    if(speed >= pclk2/32 )  return 4;
    if(speed >= pclk2/64 )  return 5;
    if(speed >= pclk2/128 ) return 6;
    return 7;
}

static inline uint8_t SPI1_ReadWriteByte(uint8_t TxData)
{
    // 等待发送缓冲区空
    while (!(SPI1_SR & SPI_SR_TXE));
    SPI1_DR = TxData;
    // 等待接收缓冲区有数据
    while (!(SPI1_SR & SPI_SR_RXNE));
    return (uint8_t)(SPI1_DR);
}



#define SPI1_BSP_DEBUG 0

static int spi_device_swap(void *tx_buf, void *rx_buf, int len)
{
    if(tx_buf == NULL && rx_buf == NULL) return -1;
    
    uint8_t *p = (uint8_t *)tx_buf;
    uint8_t *q = (uint8_t *)rx_buf;
     
    #if SPI1_BSP_DEBUG 
    if(p){ 
      printk("send data:\n\r");
      printk("TX:\n\r");
      for(int i = 0; i < len; i++){
        printk("0x%02x \n\r", p[i]);
      }
    }
    #endif


    if(p && q) {

        for(int i = 0; i < len; i++) {
            q[i] = SPI1_ReadWriteByte(p[i]);
        }
    } 
    else if(p) {
        for(int i = 0; i < len; i++) {
            SPI1_ReadWriteByte(p[i]);
        }
    }
    else if(q) {
        for(int i = 0; i < len; i++) {
            q[i] = SPI1_ReadWriteByte(0xFF);
        }
    }
    
    #if SPI1_BSP_DEBUG
    if(q){ 
      printk("receive data:\n\r");
      printk("RX:\n\r");
      for(int i = 0; i < len; i++){
        printk("0x%02x \n\r", q[i]);
      }
    }
    #endif

    return len;
}



static int spi_dev_open(struct inode *inode, struct file *file)
{
   printk("spi_dev_opern\n\r");
   return 0;
}


long spi_unlocked_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
    if (arg==NULL){
        printk("arg is null");
        return -EINVAL;
    } 
    struct spi_ioc_transfer *spi = (struct spi_ioc_transfer *) arg;
    switch(cmd)
    {
        case SPI_IOC_MESSAGE(1):
            // pr_info("setspeed:%d\n",spi->speed_hz);
            SPI1_SetSpeed(get_spi_speed(spi->speed_hz));
            return spi_device_swap(spi->tx_buf, spi->rx_buf, spi->len);
        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations spi_fops = {
    .owner = THIS_MODULE,
    .open = spi_dev_open,
    .unlocked_ioctl = spi_unlocked_ioctl,
};



static int spi1_device_init(void){
    SPI1_Init();
    
    int major = register_chrdev(0,"spidev1", &spi_fops);
    pr_info("register spi device %d",major);

    struct class * spi_class = class_create(THIS_MODULE,"spidev1");
    if(IS_ERR(spi_class)){
    pr_info("can not create spi class\n");
    }
    struct device * spi_dev = device_create(spi_class,NULL,major,NULL,"spidev1");
    if(IS_ERR(spi_dev)){
        pr_info("can not create spi device\n");
    }
    return 0;

 }
 
 device_initcall(spi1_device_init);
  
  
