#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/mempool_super_haper.h>
#include <linux/gfp_types.h>
#include <generated/autoconf.h>



#define haper_size CONFIG_KERNEL_MAIN_HEAP_SIZE*1024
static block_t haper_data[haper_size/4];
static struct haper2 memory_haper;

#define kernel_expand_heap_size CONFIG_EXPAND_HEAP_1_SIZE*1024
static block_t *haper_expand_data = CONFIG_EXPAND_HEAP_1_START_ADDR;
static struct haper2 expand_memory_haper;


#define kernel_expand_heap2_size CONFIG_EXPAND_HEAP_2_SIZE*1024
static block_t *haper_expand2_data = CONFIG_EXPAND_HEAP_2_START_ADDR;
static struct haper2 expand_memory_haper2;




struct haper2* hapers[] = {&expand_memory_haper, &expand_memory_haper2, &memory_haper};
static int haper_count = 3;

void sys_heap_init(void)
{   
    if (haper2_init(&memory_haper, haper_data, haper_size,0x00) < 0){  
        return -1;
    }
} 
void sys_expand_heap_init(void){
    haper2_init(&expand_memory_haper, haper_expand_data, kernel_expand_heap_size,0x00);   
    haper2_init(&expand_memory_haper2, haper_expand2_data, kernel_expand_heap2_size,0x00);   
}  


static void* smalloc(size_t size) {
    if (size == 0) return NULL;
    size_t use_memory_size = (size + 3) & ~3; // 四字节对齐
    void* addr = NULL;
    for (int i = 0; i < haper_count; i++) {
        addr = best_fit_alloc(hapers[i], use_memory_size);
        if (addr != NULL) return addr;
    }
    return NULL;
}

static void sfree(void* address) {
    for (int i = 0; i < haper_count; i++) {
        if (free_haper_block(hapers[i], address) > 0) return;
    }
}

int check_haper_completeness(void){
    int flag = 0;
    for (int i = 0; i < haper_count; i++) {
        flag += check_blocks_completeness(hapers[i]);
    }
    return flag;
}


size_t get_global_heap_size(void){
    size_t total_size = 0;
    for (int i = 0; i < haper_count; i++) {
        total_size += get_free_space(hapers[i]);
    }
    return total_size;
};


void print_haper_status(void){
    for (int i = 0; i < haper_count; i++) {
        show_blocks_status(hapers[i],printk);
    }
} 

void *__task_malloc(size_t size) //分配任务栈空间
{
	if (size == 0) return NULL;
	size_t use_memory_size = (size + 3) & ~3; // 四字节对齐
    void* addr = NULL;

    #if (CONFIG_EXPAND_HEAP_1_RUN_TASK)    
    addr = first_fit_alloc(&expand_memory_haper, use_memory_size);
    if(addr!= NULL)  return addr;
    #endif
    
    #if (CONFIG_EXPAND_HEAP_2_TASK_RUN)    
    addr = first_fit_alloc(&expand_memory_haper2,use_memory_size);
    if(addr!= NULL) return addr;
    #endif
    
    addr = first_fit_alloc(&memory_haper, use_memory_size);

    return addr;
}



void* __smalloc__(u32 size, gfp_t flags)
{   
    if(flags == GFP_NOWAIT){
        return __task_malloc(size);
    }
    else {
        return smalloc(size);
    }
}

void  __sfree__(void* addr){
    sfree(addr);
}



