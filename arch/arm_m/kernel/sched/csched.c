#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/sched.h>


extern void __default_Task_return_function(void);

void __init_Taskcontext(struct task_struct* task_data,uint32_t pic_offset){
    if(task_data == NULL) return;
    TaskContext* context = (TaskContext*)(&task_data->context);
    uint32_t* psp = (uint32_t*)(((uint32_t)task_data->stack_Top) & ~0x7) - 32;
    psp[15] = 0x01000000u;                  // xPSR
    psp[14] = task_data->entry;             // PC (任务入口地址)
    psp[13] = __default_Task_return_function;  // LR (返回地址)
    psp[12] = 0;                            // r12
    psp[11] = 0;                            // r3
    psp[10] = 0;                            // r2
    psp[9] = 0xFFFFFFFD;                    // r1     
    psp[8] = task_data->arg;                // r0
    psp[7] = 0;                     // r11
    psp[6] = 0;                     // r10
    psp[5] = pic_offset;            // r9
    psp[4] = 0;                     // r8
    psp[3] = 0;                     // r7
    psp[2] = 0;                     // r6
    psp[1] = 0;                     // r5
    psp[0] = (uint32_t)(psp);       // r4
    context->psp = (uint32_t)(psp);
}





