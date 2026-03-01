#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "hardware/flash.h"
#include "pico/multicore.h"



#define FLASH_START_ADDRES      0x10000000
#define FLASH_DATA_OFFERSET     1*1024*1024
#define FLASH_SIZE              4*1024*1024
#define KERNEL_RAM_START        0x20000000 + 1024*32



enum flash_page_flag {
    DIRTY,
    USING
};
struct flash_page {
    uint32_t page_number;
    char Flash_Page_Buffer[4096];    
    enum flash_page_flag FLAG;
};
static void set_page_dirty(struct flash_page *page){
    page->FLAG = DIRTY;
}

static int Set_Flash_Page_number(struct flash_page *page,uint32_t page_number){
    page->page_number = page_number;
    return 0;
}

static char* get_page_data(struct flash_page *page){
    return page->Flash_Page_Buffer;
}

static int Flash_Page_Init(struct flash_page *page){
    memset(page,0,sizeof(struct flash_page));
    page->page_number = 0;
    page->FLAG        = USING;
    return 0;
}

static int Flash_Erase_Sector(struct flash_page *page)
{
    if (page == NULL) return -1;
    flash_range_erase(page->page_number *4096, FLASH_SECTOR_SIZE);
    return 0;
}

static int Flash_Write_data(struct flash_page *page,uint32_t number)
{
    char *data = get_page_data(page);
    flash_range_program(page->page_number*4096 + number*256,
                        page->Flash_Page_Buffer+ number*256,
                        256);
    return 0;
} 

static int Flash_Write_one_Page(struct flash_page *page){
    for(int i =0;i<16;i++){
        Flash_Write_data(page,i);
    }
}

static int Flash_Write_Page(struct flash_page *page) 
{
    if(page->FLAG != DIRTY) 
    return 0;
    if((page->page_number + 1)*4096 > FLASH_SIZE) 
    return 0;
    
    uint32_t ints = save_and_disable_interrupts();
    Flash_Erase_Sector(page);
    Flash_Write_one_Page(page);
    restore_interrupts(ints);

    page->FLAG = USING;
    return 0;
}

static int Flash_Read_Page(struct flash_page *page)
{
    if((page->page_number + 1)*4096 >= FLASH_SIZE)
    return -1;
    memcpy(page->Flash_Page_Buffer, (uint32_t*)( ((uint32_t)page->page_number) *4096 + FLASH_START_ADDRES) ,4096);
    page->FLAG = USING;
}

static int UpDate_Page(struct flash_page *page,uint32_t page_number){
    if(page->FLAG == DIRTY){
        Flash_Write_Page(page);
    }
    Set_Flash_Page_number(page,page_number);
    Flash_Read_Page(page);
}

static void printstring(void *str , int len)
{
    char *p = (char *)str;
    for (int i = 0; i < len; i++) {
        if (p[i] >= 0x20 && p[i] <= 0x7E) {  // 仅打印可打印的 ASCII 字符
            printf("%c", p[i]);
        } else {
            printf(".");  // 对于不可打印的字符，用 . 来表示
        }
    }
}

static void print_memory(void *memory_addr,int len) //使用16进制打印内存
{
    uint8_t *buffer = (uint8_t *)memory_addr; 
    printf("\n\r       ");
    for(uint16_t i=0;i<16;i++){
        printf("%02d ",i);
    }        
    printf("\n\r");
    if(len < 16) len = 16;
    for(uint16_t j = 0 ; j < len/16 ; j++)
    {
        printf("0x%04X ", j*16);
        for(uint16_t i=0;i<16;i++)
        {
            printf("%02X ",buffer[j*16+i]);
        }
        printf("    ");
        printstring(buffer + j*16, 16); 
        printf("\n\r");
    }
}


static const unsigned short crc16tab[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};
  
unsigned short crc16_ccitt(const void *buf, int len)
{
	register int counter;
	register unsigned short crc = 0;
	for( counter = 0; counter < len; counter++)
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *(char *)buf++)&0x00FF];
	return crc;
}




















static int _inbyte(unsigned short timeout)
{
    absolute_time_t end_time = make_timeout_time_ms(timeout);
    while (!time_reached(end_time)) {
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            return ch;
        }
    }
    return -1;
}

static void _outbyte(char c)
{
    putchar(c);
}



#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 500
#define MAXRETRANS 25
#define TRANSMIT_XMODEM_1K

static int check(int crc, const unsigned char *buf, int sz)
{
	if (crc) {
		unsigned short crc = crc16_ccitt(buf, sz);
		unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
		if (crc == tcrc)
			return 1;
	}
	else {
		int i;
		unsigned char cks = 0;
		for (i = 0; i < sz; ++i) {
			cks += buf[i];
		}
		if (cks == buf[sz])
		return 1;
	}

	return 0;
}

static void flushinput(void)
{
	while (_inbyte(((DLY_1S)*3)>>1) >= 0)
		;
}

int xmodemReceive(unsigned char *dest, int destsz)
{
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	unsigned char *p;
	int bufsz, crc = 0;
	unsigned char trychar = 'C';
	unsigned char packetno = 1;
	int i, c, len = 0;
	int retry, retrans = MAXRETRANS;

	// 新增ZMODEM检测
    for (int zmodem_retry = 0; zmodem_retry < 3; zmodem_retry++) {
        if ((c = _inbyte(100)) == 'Z') {  // 检测ZMODEM协议头
            _outbyte(NAK);  // 强制降级到XMODEM
            sleep_ms(100);
            break;
        }
    }

	for(;;) {
		for( retry = 0; retry < 16; ++retry) {
			if (trychar) _outbyte(trychar);
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case SOH:
					bufsz = 128;
					goto start_recv;
				case STX:
					bufsz = 1024;
					goto start_recv;
				case EOT:
					flushinput();
					_outbyte(ACK);
					return len; /* normal end */
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						flushinput();
						_outbyte(ACK);
						return -1; /* canceled by remote */
					}
					break;
				default:
					break;
				}
			}
		}
		if (trychar == 'C') { trychar = NAK; continue; }
		flushinput();
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		return -2; /* sync error */

	start_recv:
		if (trychar == 'C') crc = 1;
		trychar = 0;
		p = xbuff;
		*p++ = c;
		for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
			if ((c = _inbyte(DLY_1S)) < 0) goto reject;
			*p++ = c;
		}

		if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
			(xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
			check(crc, &xbuff[3], bufsz)) {
			if (xbuff[1] == packetno)	{
				register int count = destsz - len;
				if (count > bufsz) count = bufsz;
				if (count > 0) {
					memcpy (&dest[len], &xbuff[3], count);
					len += count;
				}
				++packetno;
				retrans = MAXRETRANS+1;
			}
			if (--retrans <= 0) {
				flushinput();
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				return -3; /* too many retry error */
			}
			_outbyte(ACK);
			continue;
		}
	reject:
		flushinput();
		_outbyte(NAK);
	}
}

char buffer[1024*200];


int write_to_flash(char* buffer, int page_number, uint32_t size, struct flash_page *page)
{
    if (!buffer || !page || size == 0)
        return -1;
	#define PAGE_SIZE 4096
    uint32_t offset = 0;
    uint32_t total_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE; // ceil(size / 4096)
    for (uint32_t i = 0; i < total_pages; i++) {
        Flash_Page_Init(page);  // 清零并设置状态
        uint32_t current_page_number = page_number + i;
        Set_Flash_Page_number(page, current_page_number);
        // 计算当前 chunk 大小
        uint32_t chunk_size = (size - offset > PAGE_SIZE) ? PAGE_SIZE : (size - offset);
        // 清空缓冲区（模拟擦除）
        memset(page->Flash_Page_Buffer, 0xFF, PAGE_SIZE);
        memcpy(page->Flash_Page_Buffer, buffer + offset, chunk_size);
        set_page_dirty(page);
        Flash_Write_Page(page);  // 写入 Flash
        offset += chunk_size;
    }
    return 0;
}



struct export_node_struct{
	char *export_node_name;
	char *export_node_license;
	void *export_node_fn_address;
};


#define __EXPORT_SYMBOL(sym, license, ns)        \
	static struct export_node_struct             \
	__attribute__((__section__(".export_table")))\
	__attribute__((__used__))                    \
	sym##_export_struct = { 	                 \
		.export_node_name = #sym,			     \
		.export_node_license = license,			 \
		.export_node_fn_address = sym 			 \
	}


#define _EXPORT_SYMBOL(sym, license)	__EXPORT_SYMBOL(sym, license, "")

#define EXPORT_SYMBOL(sym)		_EXPORT_SYMBOL(sym, "BSD-3-Clause")


EXPORT_SYMBOL(putchar);
EXPORT_SYMBOL(_inbyte);

EXPORT_SYMBOL(spin_lock_blocking);
EXPORT_SYMBOL(spin_unlock);

EXPORT_SYMBOL(multicore_doorbell_claim_unused);
EXPORT_SYMBOL(multicore_doorbell_set_other_core);
EXPORT_SYMBOL(multicore_doorbell_clear_current_core);
EXPORT_SYMBOL(multicore_doorbell_irq_num);
EXPORT_SYMBOL(multicore_fifo_push_blocking);











uint32_t core1_start_stack[256];
alarm_pool_t *core1_alarm_pool = NULL;

#define SIO_CPUID_OFFSET _u(0x00000000)
#define SIO_BASE _u(0xd0000000)
static uint32_t __get_core_num__(void) {
    return (*(uint32_t *) (SIO_BASE + SIO_CPUID_OFFSET));
}

bool core1_scheduler_tick(struct repeating_timer *t) {
    return true;  
}


typedef void (*msp_init_t)();

void core1_entry() {
    printf("CPU(%d): Initializing\n", __get_core_num__());
    while (1) 
    {   
        msp_init_t function =(msp_init_t) multicore_fifo_pop_blocking();
        printf("cpu1 get function 0x%08x\n",(uint32_t)function);
        function();
        printf("cpu1 run function done\n");
    }
}


extern const struct export_node_struct __export_table_start[];
extern const struct export_node_struct __export_table_end[];


#define kernel_memork_size (512-32)*1024
struct flash_page page;

void jmp_to_start(uint32_t addr)
{
    
    printf("symbletable:0x%08x -0x%08x \n",__export_table_start,__export_table_end);
    
    if (*(volatile uint32_t*)addr == 0xFFFFFFFF) {
         printf("No valid kernel found!\n");
         while (1);
    }

    printf("jmp_to_start addr: 0x%08x\r\n", addr);

    uint32_t jmp_addr =  addr; 
    uint32_t msp_value = *(volatile uint32_t*)jmp_addr;
    uint32_t start_addr = *(volatile uint32_t*)(jmp_addr+4) ;

    printf("Application MSP: 0x%08x\r\n", msp_value);
    printf("Application Entry Point: 0x%08x\r\n", start_addr);
    printf("Set Start address : 0x%08x\r\n", start_addr);

    __asm__ volatile ("dsb");
    __asm__ volatile ("isb");

    __asm volatile ("msr msp, %0" : : "r" (msp_value) : );
    int (*kernel_main)(void*,void*,void*) = (int (*)(void*,void*,void*))(start_addr);

    printf("Jump to Second Boot!\r\n");
    kernel_main((void*)__export_table_start,(void*)__export_table_end,(void*)0);


    printf("if you get this message,you must got a bug!,check your code!\r\n");      
    while(1);
}

int main()  
{
    stdio_init_all();
    printf("\n\nGCC Version: %d.%d.%d  %s  %s\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __DATE__, __TIME__ );
    printf("---- RP2350 BootLoader ----\n");
    printf("CPU Frequency: %lu MHz ARMv%d\n", clock_get_hz(clk_sys) / 1000000, __ARM_ARCH);
    printf("XOSC Frequency: %lu Hz\n", clock_get_hz(clk_ref));  
    printf("Program Address: 0x%0x\n",FLASH_START_ADDRES+1024*1024);
    printf("Kernel Ram start:0x%0x\n",KERNEL_RAM_START);


    printf("press <space> to up date kernel \n");
    for(int i =0;i<2*1000;i++)
    {
		sleep_ms(1);
        int ch = getchar_timeout_us(0); 
		if (ch != PICO_ERROR_TIMEOUT) {
            if(ch == ' '){
                trams:
				printf("please upload bin ( use xmodem )\n");
                int err = xmodemReceive((char*)KERNEL_RAM_START,kernel_memork_size);
				if(err >= 0)
				{
					printf("file transfer completed successfully. Received %d bytes\n", err);
                    printf("write data to flash\n");
                    int page_number = err / 4096;
					write_to_flash((char*)KERNEL_RAM_START,1024/4,(page_number+1)*4096,&page);
                    printf("update\n");
                }
				else
				{
					switch(err) {
                        case -1:
                            printf("Error: Transfer canceled by remote\n");
                            break;
                        case -2:
                            printf("Error: Synchronization error - timeout\n");
                            break;
                        case -3:
                            printf("Error: Too many retries - transfer failed\n");
                            break;
                        default:
                            printf("Unknown error during transfer: %d\n", err);
                            break;
                    }
                    
                    printf("Press 'space' to try again or 'c' to boot existing kernel\n");
                    while (true) {
                        int ch = getchar_timeout_us(0); 
                        if (ch != PICO_ERROR_TIMEOUT) 
                        {
                            if(ch == ' ')
                            goto trams;
                            if(ch == 'c')
                            break;
                        } 
                    }
				}
                break;
            }
        } 
	}
    
	multicore_launch_core1_with_stack(core1_entry, (uint32_t*)((uint32_t)core1_start_stack +  sizeof(core1_start_stack)), sizeof(core1_start_stack));
    sleep_ms(5);
    printf("jump to kernel\n");     
    jmp_to_start(FLASH_START_ADDRES + 1024*1024);    
}









