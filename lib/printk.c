#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/stdarg.h>
#include <linux/string.h>
#include <linux/stddef.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/spinlock.h>
#include <generated/autoconf.h>

struct file* tty_device = NULL;
struct spinlock printk_spinlock;

int printk_service_init(void) {
    tty_device = filp_open(CONFIG_PRINTTK_USE_TTY_DEVICE, O_RDWR, 0);
    if (IS_ERR(tty_device)) {
        early_printk("tty device:%s open failed\n",CONFIG_PRINTTK_USE_TTY_DEVICE);
        return -1;
    }
    spin_lock_init(&printk_spinlock);
    return 0;
}

static void tty_write(const char *buf) {
    if (tty_device != NULL)
        kernel_write(tty_device, buf, strlen(buf), &tty_device->f_pos);
    else 
        early_printk(buf);     
}

static char printk_buf[512];

int printk_log_level_threshold = 7;  
int printk_show_log_level      = 1;  


static const char *log_level_str[] = {
    "<0>", "<1>", "<2>", "<3>", "<4>", "<5>", "<6>", "<7>"
};

void printk(int level, const char *fmt, ...){
    if(level < 0 || level > printk_log_level_threshold){
       return;
    }
    spin_lock(&printk_spinlock);
    va_list args;
    time64_t now = jiffies;
    uint32_t sec = now / HZ;
    uint32_t nsec = now % HZ ;
    if (now != 0){ 
        if(printk_show_log_level == 1)
        snprintf(printk_buf, sizeof(printk_buf), "\r[   %u.%06u] %s ", sec, nsec * (1000000/HZ),log_level_str[level]);
        else 
        snprintf(printk_buf, sizeof(printk_buf), "\r[   %u.%06u]", sec, nsec * (1000000/HZ));       
        tty_write(printk_buf);        
    }
    va_start(args, fmt);
    vsnprintf(printk_buf, sizeof(printk_buf), fmt, args);
    va_end(args);

    tty_write(printk_buf);
    spin_unlock(&printk_spinlock);
}
EXPORT_SYMBOL(printk);


static char minimal_buf[1024];
void __weak minimal_printk(const char *fmt, va_list args) {
    vsnprintf(minimal_buf, sizeof(minimal_buf), fmt, args);
    early_printk("%s", minimal_buf);
}
void __weak early_putchar(char c) {
    early_printk("%c", c);
}
EXPORT_SYMBOL(early_printk);