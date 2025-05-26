#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/gpio/gpio.h>

typedef enum {
    DISABLE = 0, 
    ENABLE = !DISABLE
} FunctionalState;

// 寄存器地址定义
#define PERIPH_BASE           (0x40000000UL)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000UL)

#define GPIOA_BASE            (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE            (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE            (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE            (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASE            (AHB1PERIPH_BASE + 0x1000UL)
#define GPIOF_BASE            (AHB1PERIPH_BASE + 0x1400UL)
#define GPIOG_BASE            (AHB1PERIPH_BASE + 0x1800UL)
#define GPIOH_BASE            (AHB1PERIPH_BASE + 0x1C00UL)
#define GPIOI_BASE            (AHB1PERIPH_BASE + 0x2000UL)
#define GPIOJ_BASE            (AHB1PERIPH_BASE + 0x2400UL)
#define GPIOK_BASE            (AHB1PERIPH_BASE + 0x2800UL)

#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800UL)

// GPIO 寄存器结构体
typedef struct {
    volatile uint32_t MODER;    // 模式寄存器
    volatile uint32_t OTYPER;   // 输出类型寄存器
    volatile uint32_t OSPEEDR;  // 输出速度寄存器
    volatile uint32_t PUPDR;    // 上拉/下拉寄存器
    volatile uint32_t IDR;      // 输入数据寄存器
    volatile uint32_t ODR;      // 输出数据寄存器
    volatile uint32_t BSRR;     // 置位/复位寄存器
    volatile uint32_t LCKR;     // 锁定寄存器
    volatile uint32_t AFR[2];   // 复用功能寄存器
} GPIO_TypeDef;

// RCC 寄存器
#define RCC_AHB1ENR           (*((volatile uint32_t *)(RCC_BASE + 0x30)))

// GPIO 指针定义
#define GPIOA                 ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                 ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC                 ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD                 ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE                 ((GPIO_TypeDef *)GPIOE_BASE)
#define GPIOF                 ((GPIO_TypeDef *)GPIOF_BASE)
#define GPIOG                 ((GPIO_TypeDef *)GPIOG_BASE)
#define GPIOH                 ((GPIO_TypeDef *)GPIOH_BASE)
#define GPIOI                 ((GPIO_TypeDef *)GPIOI_BASE)
#define GPIOJ                 ((GPIO_TypeDef *)GPIOJ_BASE)
#define GPIOK                 ((GPIO_TypeDef *)GPIOK_BASE)

// GPIO 引脚定义
#define GPIO_Pin_0            (0x0001)
#define GPIO_Pin_1            (0x0002)
#define GPIO_Pin_2            (0x0004)
#define GPIO_Pin_3            (0x0008)
#define GPIO_Pin_4            (0x0010)
#define GPIO_Pin_5            (0x0020)
#define GPIO_Pin_6            (0x0040)
#define GPIO_Pin_7            (0x0080)
#define GPIO_Pin_8            (0x0100)
#define GPIO_Pin_9            (0x0200)
#define GPIO_Pin_10           (0x0400)
#define GPIO_Pin_11           (0x0800)
#define GPIO_Pin_12           (0x1000)
#define GPIO_Pin_13           (0x2000)
#define GPIO_Pin_14           (0x4000)
#define GPIO_Pin_15           (0x8000)

// GPIO 模式定义
#define GPIO_Mode_IN          (0x00)  // 输入模式
#define GPIO_Mode_OUT         (0x01)  // 输出模式
#define GPIO_Mode_AF          (0x02)  // 复用功能模式
#define GPIO_Mode_AN          (0x03)  // 模拟模式

// 输出类型定义
#define GPIO_OType_PP         (0x00)  // 推挽输出
#define GPIO_OType_OD         (0x01)  // 开漏输出

// 上拉/下拉定义
#define GPIO_PuPd_NOPULL      (0x00)  // 无上拉下拉
#define GPIO_PuPd_UP          (0x01)  // 上拉
#define GPIO_PuPd_DOWN        (0x02)  // 下拉

// 速度定义
#define GPIO_Speed_2MHz       (0x00)  // 低速
#define GPIO_Speed_25MHz      (0x01)  // 中速
#define GPIO_Speed_50MHz      (0x02)  // 高速
#define GPIO_Speed_100MHz     (0x03)  // 超高速

// RCC AHB1 外设时钟使能
#define RCC_AHB1Periph_GPIOA  (0x00000001)
#define RCC_AHB1Periph_GPIOB  (0x00000002)
#define RCC_AHB1Periph_GPIOC  (0x00000004)
#define RCC_AHB1Periph_GPIOD  (0x00000008)
#define RCC_AHB1Periph_GPIOE  (0x00000010)
#define RCC_AHB1Periph_GPIOF  (0x00000020)
#define RCC_AHB1Periph_GPIOG  (0x00000040)
#define RCC_AHB1Periph_GPIOH  (0x00000080)
#define RCC_AHB1Periph_GPIOI  (0x00000100)
#define RCC_AHB1Periph_GPIOJ  (0x00000200)
#define RCC_AHB1Periph_GPIOK  (0x00000400)

// 新增的复用功能定义
#define GPIO_AF0              (0x0)
#define GPIO_AF1              (0x1)
#define GPIO_AF2              (0x2)
#define GPIO_AF3              (0x3)
#define GPIO_AF4              (0x4)
#define GPIO_AF5              (0x5)
#define GPIO_AF6              (0x6)
#define GPIO_AF7              (0x7)
#define GPIO_AF8              (0x8)
#define GPIO_AF9              (0x9)
#define GPIO_AF10             (0xA)
#define GPIO_AF11             (0xB)
#define GPIO_AF12             (0xC)
#define GPIO_AF13             (0xD)
#define GPIO_AF14             (0xE)
#define GPIO_AF15             (0xF)


// GPIO 初始化结构体
typedef struct {
    uint16_t GPIO_Pin;
    uint16_t GPIO_Mode;
    uint16_t GPIO_Speed;
    uint16_t GPIO_OType;
    uint16_t GPIO_PuPd;
    uint8_t GPIO_AF;    
} GPIO_InitTypeDef;

// 设备结构体
static struct gpio_device {
    uint16_t gpio_number;
    GPIO_TypeDef *gpio_type;
    GPIO_InitTypeDef GPIO_InitStructure;
};


void print_gpio_number(uint16_t pin) {
    static const char prefix[] = "export gpio: GPIO_Pin_";
    static const char suffix[] = "\n\r";
    if(pin >= GPIO_Pin_0 && pin <= GPIO_Pin_15) {
        // 计算引脚号(0-15)
        uint8_t pin_num = 0;
        while(pin >>= 1) { pin_num++; }
        char buf[5] = {0}; // 最大存储"15"+null
        if(pin_num < 10) {
            buf[0] = '0' + pin_num;
        } else {
            buf[0] = '1';
            buf[1] = '0' + (pin_num - 10);
        }       
        pr_info("%s%s%s", prefix, buf, suffix);
    } else {
        pr_info("error pin number: %d%s", pin, suffix);
    }
}

void print_gpio_type(GPIO_TypeDef *type) {
    static const char prefix[] = "gpio type is :GPIO";
    static const char suffix[] = "\n\r";
    
    if(type >= GPIOA && type <= GPIOK) {
        char port = 'A' + ((uint32_t)type - (uint32_t)GPIOA) / 0x400;
        pr_info("%s%c%s", prefix, port, suffix);
    } else {
        pr_info("error gpio type%s", suffix);
    }
}




// 设置引脚复用功能
static void GPIO_SetAF(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t AF) {
    uint32_t pinpos = 0;
    while((GPIO_Pin >> pinpos) != 1) { pinpos++; }
    
    if(pinpos < 8) {
        GPIOx->AFR[0] &= ~(0xF << (pinpos * 4));
        GPIOx->AFR[0] |= (AF << (pinpos * 4));
    } else {
        GPIOx->AFR[1] &= ~(0xF << ((pinpos-8) * 4));
        GPIOx->AFR[1] |= (AF << ((pinpos-8) * 4));
    }
}

static void GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_InitStruct) {
    uint32_t pinpos, pos, currentpin;
    
    for(pinpos = 0; pinpos < 16; pinpos++) {
        pos = (0x1 << pinpos);
        currentpin = (GPIO_InitStruct->GPIO_Pin) & pos;
        
        if(currentpin == pos) {
            // 配置模式
            GPIOx->MODER &= ~(0x3 << (pinpos * 2));
            GPIOx->MODER |= (GPIO_InitStruct->GPIO_Mode << (pinpos * 2));
            
            // 如果是复用模式或输出模式，配置其他参数
            if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_OUT || 
               GPIO_InitStruct->GPIO_Mode == GPIO_Mode_AF) {
                // 配置输出类型
                GPIOx->OTYPER &= ~(0x1 << pinpos);
                GPIOx->OTYPER |= (GPIO_InitStruct->GPIO_OType << pinpos);
                
                // 配置速度
                GPIOx->OSPEEDR &= ~(0x3 << (pinpos * 2));
                GPIOx->OSPEEDR |= (GPIO_InitStruct->GPIO_Speed << (pinpos * 2));
            }
            
            // 配置上拉/下拉
            GPIOx->PUPDR &= ~(0x3 << (pinpos * 2));
            GPIOx->PUPDR |= (GPIO_InitStruct->GPIO_PuPd << (pinpos * 2));
            
            // 如果是复用模式，配置复用功能
            if(GPIO_InitStruct->GPIO_Mode == GPIO_Mode_AF) {
                GPIO_SetAF(GPIOx, pos, GPIO_InitStruct->GPIO_AF);
            }
        }
    }
}

// 设置GPIO引脚
static void GPIO_SetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIOx->BSRR = GPIO_Pin;
}

// 复位GPIO引脚
static void GPIO_ResetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16;
}

// 读取输入数据位
static uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    return ((GPIOx->IDR & GPIO_Pin) != 0) ? 1 : 0;
}

// 使能GPIO时钟
static void RCC_AHB1PeriphClockCmd(uint32_t RCC_AHB1Periph, uint8_t NewState) {
    if(NewState!= DISABLE) {
        RCC_AHB1ENR |= RCC_AHB1Periph;
    } else {
        RCC_AHB1ENR &= ~RCC_AHB1Periph;
    }
}

// 获取GPIO编号
static uint16_t gpio_pins[16] = {
    GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3,
    GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7,
    GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_10, GPIO_Pin_11,
    GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15
};

static uint16_t get_gpio_number(uint16_t pin) {
    int i = pin % 16;
    return gpio_pins[i];
}

// 获取GPIO类型
static GPIO_TypeDef* gpio_Types[11] = {
    GPIOA, GPIOB, GPIOC, GPIOD,
    GPIOE, GPIOF, GPIOG, GPIOH,
    GPIOI, GPIOJ, GPIOK
};

static GPIO_TypeDef *get_gpio_type(uint16_t pin) {
    int i = pin / 16;
    if(i < 11) {
        return gpio_Types[i];
    }
    else return NULL;
}

// 获取RCC时钟源
static uint32_t Get_APB_RCC_Source(GPIO_TypeDef *type) {
    if(type == GPIOA)      return RCC_AHB1Periph_GPIOA;
    else if(type == GPIOB) return RCC_AHB1Periph_GPIOB;
    else if(type == GPIOC) return RCC_AHB1Periph_GPIOC;
    else if(type == GPIOD) return RCC_AHB1Periph_GPIOD;
    else if(type == GPIOE) return RCC_AHB1Periph_GPIOE;
    else if(type == GPIOF) return RCC_AHB1Periph_GPIOF;
    else if(type == GPIOG) return RCC_AHB1Periph_GPIOG;
    else if(type == GPIOH) return RCC_AHB1Periph_GPIOH;
    else if(type == GPIOI) return RCC_AHB1Periph_GPIOI;
    else if(type == GPIOJ) return RCC_AHB1Periph_GPIOJ;
    else if(type == GPIOK) return RCC_AHB1Periph_GPIOK;
    return 0xffffffff;
}


// 获取引脚当前复用功能
static uint8_t GPIO_GetAF(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    uint32_t pinpos = 0;
    while((GPIO_Pin >> pinpos) != 1) { pinpos++; }
    
    if(pinpos < 8) {
        return (GPIOx->AFR[0] >> (pinpos * 4)) & 0xF;
    } else {
        return (GPIOx->AFR[1] >> ((pinpos-8) * 4)) & 0xF;
    }
}



// 文件操作函数
static int gpio_open(struct inode *inode, struct file *file) {
    struct gpio_device *device = kmalloc(sizeof(struct gpio_device), GFP_KERNEL);
    if(device == NULL) return -ENOMEM;
    device->gpio_number = 0xffff;
    device->gpio_type = NULL;
    file->private_data = device;
    pr_info("open gpios device\n\r");
    return 0;
}

static int gpio_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char buf_local = ((char*)buf)[0];
    struct gpio_device *device = file->private_data;

    if(device->gpio_number == 0xffff) {
        pr_info("please use ioctl to set pin number first\n\r");
        return -EINVAL;
    }

    if(buf_local == '0') {
        GPIO_ResetBits(device->gpio_type, device->gpio_number);
    }
    else if(buf_local == '1') {
        GPIO_SetBits(device->gpio_type, device->gpio_number);
    }
    else {
        pr_info("error input\n\r");
        return -EINVAL;
    }

    return len;
}

static int gpio_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    struct gpio_device *device = file->private_data;
    if(device->gpio_number == 0xffff) {
        pr_info("please use ioctl to set pin number first\n\r");
        return -EINVAL;
    }
    int value = GPIO_ReadInputDataBit(device->gpio_type, device->gpio_number);
    if(value == 0) {
        copy_to_user(buf, "0", 1);
        pr_info("gpio input value is 0\n\r");
    }
    else {
        copy_to_user(buf, "1", 1);
        pr_info("gpio input value is 1\n\r");
    }
    return len;
}

static int gpio_release(struct inode *inode, struct file *file) {
    struct gpio_device *device = file->private_data;
    kfree(device);
    return 0;
}


static long compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct gpio_device *device = file->private_data;
    if(cmd != GPIO_SET_PIN_NUMBER && device->gpio_number == 0xffff) {
        pr_info("please set pin number first\n\r");
        return -EINVAL;
    }

    uint16_t pin_number = get_gpio_number(arg);
    GPIO_TypeDef *gpio_type = get_gpio_type(arg);
    uint32_t APB_RCC_Source = Get_APB_RCC_Source(gpio_type);

    switch(cmd) {
        case GPIO_SET_PIN_NUMBER:
            if(gpio_type == NULL) {
                pr_info("gpio type is not useful\n\r");
                return -EINVAL;
            }
            if(APB_RCC_Source == 0xffffffff) {
                pr_info("APB_RCC_Source is not useful\n\r");
                return -EINVAL;
            }
            print_gpio_number(pin_number);
            print_gpio_type(gpio_type);
            
            RCC_AHB1PeriphClockCmd(APB_RCC_Source, ENABLE);
            device->gpio_number = pin_number;
            device->gpio_type = gpio_type;
            device->GPIO_InitStructure.GPIO_Pin = pin_number;
            device->GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            device->GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            device->GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            device->GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            device->GPIO_InitStructure.GPIO_AF = GPIO_AF0; // 默认AF0
            GPIO_Init(gpio_type, &device->GPIO_InitStructure);
            return 0;
            
        case GPIO_PULL_UP:
            device->GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_Init(gpio_type, &device->GPIO_InitStructure);
            return 0;
            
        case GPIO_PULL_DOWN:
            device->GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
            GPIO_Init(gpio_type, &device->GPIO_InitStructure);
            return 0;
            
        case GPIO_INPUT:
            device->GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_Init(gpio_type, &device->GPIO_InitStructure);
            return 0;
            
        case GPIO_OUTPUT:
            device->GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_Init(gpio_type, &device->GPIO_InitStructure);
            return 0;
            
        case GPIO_HIGH:
            GPIO_SetBits(gpio_type, pin_number);
            return 0;
            
        case GPIO_LOW:
            GPIO_ResetBits(gpio_type, pin_number);
            return 0;
            
        case GPIO_SET_AF: {
            // 设置复用功能，参数格式：高16位为AF编号，低16位为pin编号
            uint8_t af = (arg >> 16) & 0xF;
            device->GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
            device->GPIO_InitStructure.GPIO_AF = af;
            GPIO_Init(gpio_type, &device->GPIO_InitStructure);
            pr_info("Set pin AF to %d\n\r", af);
            return 0;
        }
            
        case GPIO_GET_AF: {
            // 获取当前复用功能
            uint8_t af = GPIO_GetAF(gpio_type, pin_number);
            pr_info("Current pin AF: %d\n\r", af);
            return af; // 返回当前AF值
        }
            
        default:
            return -EINVAL;
    }
}

static struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .open = gpio_open,
    .write = gpio_write,
    .read = gpio_read,
    .unlocked_ioctl = compat_ioctl,
    .release = gpio_release,
};

static int __init gpio_init(void)
{
    int major = register_chrdev(0, "gpios", &gpio_fops);
	pr_info ( "register gpio (%d) \n" , major);

    struct class* gpio_class = class_create(THIS_MODULE,"gpios");
    if(IS_ERR(gpio_class)){
        pr_info("can not create gpio_class\n");
    }
    struct device * dev = device_create(gpio_class,NULL,major,NULL,"gpios");
    if(IS_ERR(gpio_class)){
        pr_info("can not create gpios device\n");
    }
    return 0;
}


device_initcall(gpio_init);

