#include <linux/gpio_subsystem.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/gpio/gpio.h>

static struct file *gpio_file = NULL;

int gpio_subsystem_init(void)
{
    gpio_file = filp_open("/dev/gpios", O_RDWR, 0);
    if (IS_ERR(gpio_file)) {
        printk("Failed to open GPIO device\n");
        return PTR_ERR(gpio_file);
    }
    return 0;
}


void gpio_subsystem_exit(void)
{
    if (gpio_file) {
        filp_close(gpio_file, NULL);
        gpio_file = NULL;
    }
}

// 请求GPIO
int gpio_request(unsigned int gpio, const char *label)
{
    if (!gpio_file) return -ENODEV;
    
    int ret = vfs_ioctl(gpio_file, GPIO_SET_PIN_NUMBER, gpio);
    if (ret < 0) {
        printk("Failed to request GPIO %d: %d\n", gpio, ret);
        return ret;
    }
    
    printk("Requested GPIO %d (%s)\n", gpio, label);
    return 0;
}

// 释放GPIO
void gpio_free(unsigned int gpio)
{
    // 在您的实现中可能需要额外的清理工作
    printk("Freed GPIO %d\n", gpio);
}

// 设置为输入模式
int gpio_direction_input(unsigned int gpio)
{
    if (!gpio_file) return -ENODEV;
    return vfs_ioctl(gpio_file, GPIO_INPUT, gpio);
}

// 设置为输出模式并设置初始值
int gpio_direction_output(unsigned int gpio, int value)
{
    if (!gpio_file) return -ENODEV;
    
    int ret = vfs_ioctl(gpio_file, GPIO_OUTPUT, gpio);
    if (ret < 0) return ret;
    
    return gpio_set_value(gpio, value);
}

// 设置GPIO值
int gpio_set_value(unsigned int gpio, int value)
{
    if (!gpio_file) return -ENODEV;
    
    char buf[2] = { value ? '1' : '0', 0 };
    return kernel_write(gpio_file, buf, 1, 0);
}

// 获取GPIO值
int gpio_get_value(unsigned int gpio)
{
    if (!gpio_file) return -ENODEV;
    
    char buf[2] = {0};
    int ret = kernel_read(gpio_file, buf, 1, 0);
    if (ret < 0) return ret;
    
    return (buf[0] == '1') ? 1 : 0;
}

// 设置上下拉
int gpio_set_pull(unsigned int gpio, gpio_pull_t pull)
{
    if (!gpio_file) return -ENODEV;
    
    switch (pull) {
        case GPIO_PULL_UP:   return vfs_ioctl(gpio_file, GPIO_PULL_UP, gpio);
        case GPIO_PULL_DOWN: return vfs_ioctl(gpio_file, GPIO_PULL_DOWN, gpio);
        default:             return vfs_ioctl(gpio_file, GPIO_PULL_NONE, gpio);
    }
}

// 设置速度
int gpio_set_speed(unsigned int gpio, gpio_speed_t speed)
{
    // 在您的底层驱动中需要实现对应的IOCTL命令
    return -ENOSYS; // 暂未实现
}

// 设置输出类型
int gpio_set_output_type(unsigned int gpio, gpio_output_type_t type)
{
    // 在您的底层驱动中需要实现对应的IOCTL命令
    return -ENOSYS; // 暂未实现
}

// 设置复用功能
int gpio_set_alt_function(unsigned int gpio, gpio_alt_func_t af)
{
    if (!gpio_file) return -ENODEV;
    
    // 将AF编号放在高16位，GPIO编号在低16位
    unsigned long arg = ((af & 0xF) << 16) | (gpio & 0xFFFF);
    return vfs_ioctl(gpio_file, GPIO_SET_AF, arg);
}





