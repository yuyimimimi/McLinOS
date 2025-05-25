#ifndef __CONTEXT_H__
#define __CONTEXT_H__                


extern void __init_Taskcontext(struct task_struct* task_data,uint32_t pic_offset);
static void init_task_context(struct task_struct* task_data,uint32_t pic_offset){
    __init_Taskcontext(task_data,pic_offset);
} 

extern int __get_task_using_cpu_core(void);

static int get_task_using_cpu_core(void){
     return  __get_task_using_cpu_core();
}

static void save_context(struct task_struct *this_task,uint32_t data){
        this_task->context.psp = data;
}
static uint32_t load_task(struct task_struct *this_task) {
    return this_task->context.psp;
};


int __system_call(int sys_no, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5);
static void* user_system_call(int sys_no, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5){
    __system_call(sys_no,arg0, arg1, arg2, arg3,arg4,arg5);
}


#endif