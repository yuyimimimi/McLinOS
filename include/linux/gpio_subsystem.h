#ifndef __GPIO_SUBSYSTEM_H__
#define __GPIO_SUBSYSTEM_H__

#include <linux/types.h>

// GPIO方向定义
typedef enum {
    GPIO_DIR_IN,        // 输入
    GPIO_DIR_OUT,       // 输出
    GPIO_DIR_AF,        // 复用功能
    GPIO_DIR_ANALOG     // 模拟
} gpio_direction_t;

// GPIO上下拉配置
typedef enum {
    GPIO_PULL_NONE,     // 无上下拉
    GPIO_PULL_UP,       // 上拉
    GPIO_PULL_DOWN      // 下拉
} gpio_pull_t;

// GPIO驱动强度/速度
typedef enum {
    GPIO_SPEED_LOW,     // 低速
    GPIO_SPEED_MEDIUM,  // 中速
    GPIO_SPEED_HIGH,    // 高速
    GPIO_SPEED_VERYHIGH // 超高速
} gpio_speed_t;

// GPIO输出类型
typedef enum {
    GPIO_OUTPUT_PP,     // 推挽输出
    GPIO_OUTPUT_OD      // 开漏输出
} gpio_output_type_t;

// GPIO复用功能
typedef enum {
    GPIO_AF0 = 0,
    GPIO_AF1,
    GPIO_AF2,
    GPIO_AF3,
    GPIO_AF4,
    GPIO_AF5,
    GPIO_AF6,
    GPIO_AF7,
    GPIO_AF8,
    GPIO_AF9,
    GPIO_AF10,
    GPIO_AF11,
    GPIO_AF12,
    GPIO_AF13,
    GPIO_AF14,
    GPIO_AF15
} gpio_alt_func_t;

// GPIO编号计算宏
#define GPIO_NUM(port, pin)  (((port - 'A') << 4) | (pin))

// API函数
int gpio_request(unsigned int gpio, const char *label);
void gpio_free(unsigned int gpio);
int gpio_direction_input(unsigned int gpio);
int gpio_direction_output(unsigned int gpio, int value);
int gpio_set_value(unsigned int gpio, int value);
int gpio_get_value(unsigned int gpio);
int gpio_set_pull(unsigned int gpio, gpio_pull_t pull);
int gpio_set_speed(unsigned int gpio, gpio_speed_t speed);
int gpio_set_output_type(unsigned int gpio, gpio_output_type_t type);
int gpio_set_alt_function(unsigned int gpio, gpio_alt_func_t af);

#endif // __GPIO_SUBSYSTEM_H__

