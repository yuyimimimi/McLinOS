#include <stdint.h>
#include <stddef.h>

typedef void(*__start)(void);
typedef struct {
    uint32_t magic;         
    uint32_t kernel_ver;
    uint32_t cmd;         
    uint32_t arch_flags;
    uint32_t stack_size;
    __start  start;    
    uint32_t got_start;
    uint32_t got_end;
    uint32_t program_size;
} AppMeta;


int main(int argc,char*argv[]);
static void reset_handler(char* argv[]){
    int argc = 0;
    while (argv[argc] != NULL){
        argc++;
    }
    main(argc,argv);
}


extern uint32_t _got_start[];
extern uint32_t _got_end[];
extern uint32_t __end_program[];

AppMeta __attribute__((__section__(".appmeta"))) head = {
    .magic        =  12345678,
    .kernel_ver   =  0,
    .cmd          =  1,
    .arch_flags   =  7,
    .stack_size   =  8*1024,                
    .start        = reset_handler,
    .got_start    = _got_start,
    .got_end      = _got_end,
    .program_size = __end_program,
};








