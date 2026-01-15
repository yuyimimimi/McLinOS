#include <linux/stdarg.h>
typedef void(*_put_char_t)(char c);
extern _put_char_t _put_char;

void early_printk(const char *fmt, ...){
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    for(int i = 0; buf[i]!= '\0'; i++){
        _put_char(buf[i]);
        if(buf[i] == '\n')
        _put_char('\r');
    }
} 


