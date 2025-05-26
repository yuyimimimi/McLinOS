#ifndef __W25QXX_H__
#define __W25QXX_H__


struct w25qxx_dev
{
    u8 status; 
    struct file *spi_dev;
    struct file *gpio_cs_dev;
    struct spi_ioc_transfer xfer;
    uint16_t id;
    uint32_t size;
    int (*send_data)(struct w25qxx_dev *w25qxx_dev,u8 *data,int len);
    int (*read_data)(struct w25qxx_dev *w25qxx_dev,u8 *data,int len);
    void (*send_start)(struct w25qxx_dev *w25qxx_dev);
    void (*send_stop)(struct w25qxx_dev *w25qxx_dev);
};

struct w25qxx_dev *new_w25qxx_dev(char *spi_dev,int cs_pin_num);
#define W25QXX_SECTOR_SIZE 4096
int filp_close(struct file *file, fl_owner_t id);

static int cs_pin_init(struct w25qxx_dev *w25qxx_dev,char *gpio_dev_name ,int pin_num)
{
    if(pin_num == -1)return 0;
    
    w25qxx_dev->gpio_cs_dev = filp_open(gpio_dev_name, O_RDWR, 0);
    if(IS_ERR(w25qxx_dev->gpio_cs_dev)){
        pr_info("open file failed\n\r");
        return -1;
    }
    pr_info("set cs ping use %d\n\r",pin_num);
    int i = vfs_ioctl(w25qxx_dev->gpio_cs_dev,GPIO_SET_PIN_NUMBER,pin_num);
    if(i < 0){
        pr_info("set pin number failed\n\r");
        return -1;
    }
    vfs_ioctl(w25qxx_dev->gpio_cs_dev,GPIO_OUTPUT   ,0);
    return 0;
}
 

static int spi_write_enable(struct w25qxx_dev *w25qxx_dev,char *name ,uint32_t speed)
{
    w25qxx_dev->spi_dev = filp_open("/dev/spidev1", O_RDWR, 0);
    if(IS_ERR(w25qxx_dev->spi_dev)){
        pr_info("open file failed\n\r");
        return -1;
    }
    w25qxx_dev->xfer.speed_hz = speed;
    w25qxx_dev->xfer.len = 1;
    w25qxx_dev->xfer.delay_usecs = 0;
    w25qxx_dev->xfer.bits_per_word = 8;
    w25qxx_dev->xfer.cs_change = 0;
}


static int w25qxx_device_init(struct w25qxx_dev *dev,char *spi_dev,int cs_pin_num)
{
    dev->status = cs_pin_init(dev, "/dev/gpios",cs_pin_num);
    if(dev->status < 0){
        pr_info("cs_pin_init failed\n\r");
        return -1;
    }
    dev->status = spi_write_enable(dev,spi_dev, 1000000);
    if(dev->status < 0){
        pr_info("spi_write_enable failed\n\r");
        filp_close(dev->gpio_cs_dev, NULL);
        return -1;
    }
    return 0;
}



static  void set_cs_pin_high(struct w25qxx_dev *w25qxx_dev){
    int ret = kernel_write(w25qxx_dev->gpio_cs_dev, "1", 1, 0);
    if(ret < 0){
       pr_info("write error 1 \n\r");
    }
}

static  void set_cs_pin_low(struct w25qxx_dev *w25qxx_dev){
   int ret = kernel_write(w25qxx_dev->gpio_cs_dev, "0", 1, 0);
   if(ret < 0){
      pr_info("write error 2 \n\r");
   }
}

static int w25qxx_send_data(struct w25qxx_dev *dev,u8 *data,int len)
{
    dev->xfer.tx_buf = data;
    dev->xfer.rx_buf = NULL;
    dev->xfer.len = len;
    int i = vfs_ioctl(dev->spi_dev,SPI_IOC_MESSAGE(1),&dev->xfer);
    return i;
}
static int w25qxx_read_data(struct w25qxx_dev *dev,u8 *data,int len)
{
    dev->xfer.tx_buf = NULL;
    dev->xfer.rx_buf = data;
    dev->xfer.len = len;
    int i = vfs_ioctl(dev->spi_dev,SPI_IOC_MESSAGE(1),&dev->xfer);
    return i;
}



static int w25qxx_read_id(struct w25qxx_dev *dev)
{
    uint8_t cmd2 = 0x9F;
    uint8_t id[3] = {0,0,0}; 
    dev->send_start(dev);
    dev->send_data(dev,&cmd2,1);
    dev->read_data(dev,id,3);
    dev->send_stop(dev);
    dev->size = 0;
    if(id[2]  >= 0x10 && id[2] <= 0x25){
        dev->size  = 1 << id[2];
    }
    if(dev->size == 0){
        return -1;
    }

    pr_info("FLASH JEDEC ID: 0x%02X 0x%02X 0x%02X ,Detected Flash Size: %d KB (%d MB)\r\n", id[0], id[1], id[2], dev->size / 1024, dev->size / (1024 * 1024));
    u8 cmd[4] = {0x90,0x00,0x00,0x00};
    dev->send_start(dev);
    dev->send_data(dev,cmd,4);
    dev->read_data(dev,&dev->id,2);
    dev->send_stop(dev);
    return 0;
}


struct w25qxx_dev *new_w25qxx_dev(char *spi_dev,int cs_pin_num)
{
    struct w25qxx_dev *w25qxx_dev = kmalloc(sizeof(struct w25qxx_dev), GFP_KERNEL);
    if(w25qxx_dev == NULL){
        pr_info("malloc failed\n\r");
        return NULL;
    }
    w25qxx_dev->send_data = w25qxx_send_data;
    w25qxx_dev->read_data = w25qxx_read_data;
    w25qxx_dev->send_start = set_cs_pin_low;
    w25qxx_dev->send_stop = set_cs_pin_high;
    w25qxx_dev->status = w25qxx_device_init(w25qxx_dev,spi_dev,cs_pin_num);
    
    if( w25qxx_read_id(w25qxx_dev) < 0 )
    {
        pr_err("can not read w25qxx spi flash \n");
        filp_close(w25qxx_dev->gpio_cs_dev,NULL);
        filp_close(w25qxx_dev->spi_dev,NULL);
        kfree(w25qxx_dev);
        return NULL;
    }
    
    if(w25qxx_dev->status < 0){
        kfree(w25qxx_dev);
        return NULL;
    }
    return w25qxx_dev;
}

static void W25Qxx_write_enable(struct w25qxx_dev *dev)
{
    dev->send_start(dev);
    uint8_t cmd = 0x06;
    dev->send_data(dev,&cmd,1);
    dev->send_stop(dev);
}

static void W25Qxx_wait_busy(struct w25qxx_dev *dev)
{
    dev->send_start(dev);
    uint8_t cmd[2] = {0x05, 0xff};
    do{             
        cmd[1] = 0xff;
        dev->send_data(dev,cmd,2);
        dev->read_data(dev,cmd,2);   
    }while((cmd[1] & 0x01) == 0x01);
    dev->send_stop(dev);     
}

static void W25Qxx_erase_sector(struct w25qxx_dev *dev,u32 sector_num)
{
    sector_num *= 4096;
    W25Qxx_write_enable(dev);
    dev->send_start(dev);
    uint8_t cmd[4] = {0x20, (sector_num >> 16) & 0xff, (sector_num >> 8) & 0xff, sector_num & 0xff};
    dev->send_data(dev,cmd,4);
    dev->send_stop(dev);
}
static void W25Qxx_write(struct w25qxx_dev *dev,uint8_t* buffer, uint32_t addr, uint16_t numbyte)
{
    W25Qxx_wait_busy(dev);
    W25Qxx_write_enable(dev);
    dev->send_start(dev);
    uint8_t cmd[4] = {0x02, (addr >> 16) & 0xff, (addr >> 8) & 0xff, addr & 0xff};
    dev->send_data(dev,cmd,4);
    dev->send_data(dev,buffer,numbyte);
    dev->send_stop(dev);
}

static void W25Qxx_read(struct w25qxx_dev *dev,uint8_t* buffer, uint32_t addr, uint16_t numbyte)
{
    W25Qxx_wait_busy(dev);
    dev->send_start(dev);
    uint8_t cmd[4] = {0x03, (addr >> 16) & 0xff, (addr >> 8) & 0xff, addr & 0xff};
    dev->send_data(dev,cmd,4);
    dev->read_data(dev,buffer,numbyte);
    dev->send_stop(dev);
}

static int  W25Qxx_device_write_page(struct w25qxx_dev *dev,void *buf, int len,uint32_t page_4k)
{
    W25Qxx_erase_sector(dev,page_4k);
    int times  = len / 256;
    int offset = page_4k*4096;
    for(int i =0;i<times;i++){
        W25Qxx_write(dev,(uint8_t *)buf + i*256, offset ,256);            
        offset += 256;
    }
    return len;
}

static int  W25Qxx_device_read_page(struct w25qxx_dev *dev,void *buf, int len,uint32_t page_512b){
    W25Qxx_read(dev,buf,page_512b*512,len);
    return len;
}

#endif /* __W25QXX_H__ */