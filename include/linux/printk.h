/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KERNEL_PRINTK__
#define __KERNEL_PRINTK__
#include <linux/types.h>

#define KERN_INFO
#define KERN_WARNING
#define KERN_ERR






extern void early_printk(const char *fmt, ...);


extern void printk(const char *fmt, ...);

#define pr_info(fmt, ...) printk(KERN_INFO fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...) printk(KERN_WARNING fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) printk(KERN_ERR fmt, ##__VA_ARGS__)


static void printstring(void *str , int len)
{
    char *p = (char *)str;
    for (int i = 0; i < len; i++) {
        if (p[i] >= 0x20 && p[i] <= 0x7E) {  // 仅打印可打印的 ASCII 字符
            early_printk("%c", p[i]);
        } else {
            early_printk(".");  // 对于不可打印的字符，用 . 来表示
        }
    }
}

static void print_memory(void *memory_addr,int len) //使用16进制打印内存
{
    uint8_t *buffer = (uint8_t *)memory_addr; 
    early_printk("\n\r       ");
    for(uint16_t i=0;i<16;i++){
        early_printk("%02d ",i);
    }        
    early_printk("\n\r");

    for(uint16_t j = 0 ; j < len/16 ; j++)
    {
        early_printk("0x%04X ", j*16);
        for(uint16_t i=0;i<16;i++)
        {
            early_printk("%02X ",buffer[j*16+i]);
        }
        early_printk("    ");
        printstring(buffer + j*16, 16); 
        early_printk("\n\r");
    }
}



#endif





