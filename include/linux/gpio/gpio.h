#ifndef __GPIO_H__
#define __GPIO_H__
//这个文件是一个临时文件，pinctrl完后后会被淘汰

#define GPIO_INPUT          (0x01 << 0)
#define GPIO_OUTPUT         (0x01 << 1)
#define GPIO_LOW            (0x01 << 2)
#define GPIO_HIGH           (0x01 << 3)
#define GPIO_PULL_UP        (0x01 << 4)
#define GPIO_PULL_DOWN      (0x01 << 5)
#define GPIO_SET_PIN_NUMBER (0x01 << 6)
#define GPIO_SET_AF         (0x100)  // 设置复用功能
#define GPIO_GET_AF         (0x101)  // 获取当前复用功能

#endif /* __GPIO_H__ */ 