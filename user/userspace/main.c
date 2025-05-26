#include <stdint.h>
#define SYS_PRINT 0
typedef void(*__start)(void);

typedef struct {
    uint32_t magic;
    uint32_t kernel_ver;
    uint32_t cmd;    
    uint32_t arch_flags;
    uint32_t stank_size;
    __start  start; // 注意：这里是偏移值
    uint32_t got_start;
    uint32_t got_end;
} AppMeta;


extern uint32_t _got_start[];
extern uint32_t _got_end[];

 

extern int main(void);
void reset_handler(void) {
    main();
}


__attribute__((__section__(".appmeta")))
const AppMeta app_meta = {
    .magic      = 12345678,
    .kernel_ver = 0,
    .cmd        = 0,
    .arch_flags = 7,    
    .stank_size = 1024 * 4,
    .start     = reset_handler,
    .got_start = _got_start,
    .got_end   = _got_end
};


struct systemcall {
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r7;
}__attribute__((packed));

int __system_call(int sys_no, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5)
{
    register int r0 __asm("r0") = arg0;
    register int r1 __asm("r1") = arg1;
    register int r2 __asm("r2") = arg2;

    struct systemcall syscall_argvs = {
        .r3 = arg3,
        .r4 = arg4,
        .r5 = arg5,
        .r7 = sys_no,
    };
    register int r3 __asm("r3") = &syscall_argvs;
    __asm__ volatile (
        "svc 0"  
        : "=r" (r0)
        : "r" (r0), "r" (r1), "r" (r2), "r" (r3)
        : "memory"
    );
    return r0;
}

void pr_info(const char* str) 
{
    __system_call(SYS_PRINT, (int)str, 0, 0, 0, 0, 0);
}

int main(void) 
{
    for(int i = 0;i<10;i++)
    {
        pr_info("Hello from user app!\n");        
    }
    return 0;
}