#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/sched.h>


extern void __default_Task_return_function(void);

void __init_Taskcontext(struct task_struct* task_data){
    if(task_data == NULL) return;
    TaskContext* context = (TaskContext*)(&task_data->context);
    uint32_t* psp = (uint32_t*)(((uint32_t)task_data->stack_Top) & ~0x7) - 16;
    psp[15] = 0x01000000u; 
    psp[14] = task_data->entry; 
    psp[13] = __default_Task_return_function;  
    psp[12] = 0;         
    psp[11] = 0;         
    psp[10] = 0;         
    psp[9] = 0xFFFFFFFD;       
    psp[8] = task_data->arg; 
    psp[7] = 0;
    psp[6] = 0;
    psp[5] = 0;
    psp[4] = 0;
    psp[3] = 0;
    psp[2] = 0;
    psp[1] = 0;
    psp[0] = (uint32_t)(psp);
    context->psp = (uint32_t)(psp);
} 

int __get_task_using_cpu_core(void){
    return 0;
}



