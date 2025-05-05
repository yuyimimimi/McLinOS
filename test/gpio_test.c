
void gpio_test(void)
{
    printk("GPIO test\n\r");
    struct file *fp = filp_open("/dev/gpios", O_RDWR, 0);
    if(IS_ERR(fp)){
        printk("open file failed\n\r");
        return;
    }
    int i = vfs_ioctl(fp,GPIO_SET_PIN_NUMBER,18);
    if(i < 0){
        printk("set pin number failed\n\r");
        return;
    }
    vfs_ioctl(fp,GPIO_OUTPUT,0);
    kernel_write(fp, "1", 1, 0);
}