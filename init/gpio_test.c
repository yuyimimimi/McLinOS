#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio/gpio.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>

void gpio_test( )
{
    pr_info("GPIO test\n\r");
    struct file *fp = filp_open("/dev/gpios", O_RDWR, 0);
    if(IS_ERR(fp))
    {
        pr_info("open file failed\n\r");
        return;
    }
    int i = vfs_ioctl(fp,GPIO_SET_PIN_NUMBER,18);
    if(i < 0){
        pr_info("set pin number failed\n\r");
        return;
    }
    vfs_ioctl(fp,GPIO_OUTPUT,0);
    while(1)
   {
        kernel_write(fp, "1", 1, 0);
        msleep(5);     
        kernel_write(fp, "0", 1, 0);        
        msleep(995);
    }

}
static void gpio_task()
{
    kthread_run(gpio_test,NULL,"gpio_tset");
}

late_initcall(gpio_task);
