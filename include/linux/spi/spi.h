#ifndef _UAPI_LINUX_SPI_SPI_H
#define _UAPI_LINUX_SPI_SPI_H

#include <linux/types.h>

#define SPI_IOC_MESSAGE(x)       x




struct spi_ioc_transfer  {
    void* tx_buf;
    void* rx_buf;
    __u32 len;
    __u32 speed_hz;
    __u16 delay_usecs; 
    __u8 bits_per_word;
    __u32 mode;
    __u8 cs_change;
}__attribute__((__packed__));



#endif // !1


