#include <linux/sched.h>
#include <linux/sys_call.h>
#include <linux/export.h>

void block_scheduler(struct scheduler *s){
    if(s == NULL){
        s = get_current_scheduler();
    }
    s->s_flag = SCHEDULER_BLOCKED;
}
void run_scheduler(struct scheduler *s){
    if(s == NULL){
        s = get_current_scheduler(); 
    }
    s->s_flag = SCHEDULER_RUN;
}

struct scheduler *get_current_scheduler(void){
    return   get_scheduler_by_cpu_core_id(get_task_using_cpu_core());
}

struct task_struct* get_current_task(void){
    return   get_scheduler_by_cpu_core_id(get_task_using_cpu_core())->current_task;
}

static int  Scheduler_Lock1 = 0;
void scheduler_start(){
    Scheduler_Lock1++;
}

static enum scheduler_block_flag Scheduler_Lock = SCHEDULER_RUN; //全局调度锁
void start_all_scheduler(){
    Scheduler_Lock = SCHEDULER_RUN;
}
void stop_all_scheduler(){
    Scheduler_Lock = SCHEDULER_BLOCKED;
}


int i_sched(){
    if(Scheduler_Lock == SCHEDULER_RUN && Scheduler_Lock1 == CONFIG_CPU_NUM ){
        __sched();
        return 0;
    }
    return -1;
}


void sched(void){
    volatile int j  = 0;      
    while ( user_system_call(158,NULL,NULL,NULL,NULL,NULL,NULL) < 0){  
        for(int i =0;i<10000;i++){j++;} //调度器阻塞无法切换，那就只能阻塞了
    }
}


void __delay(uint32_t time)
{
    struct task_struct* task = get_current_task();
    task->block_time = time;
    task->state = TASK_WAITING;
    sched();
}





