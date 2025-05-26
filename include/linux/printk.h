/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KERNEL_PRINTK__
#define __KERNEL_PRINTK__
#include <linux/types.h>

#define KERN_EMERG    0,
#define KERN_ALERT    1,
#define KERN_CRIT     2,
#define KERN_ERR      3,
#define KERN_WARNING  4,
#define KERN_NOTICE   5,
#define KERN_INFO     6,
#define KERN_DEBUG    7,






extern void early_printk(const char *fmt, ...);


extern void printk(int leave, const char *fmt, ...);

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
    if(len < 16) len = 16;
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





