#include <linux/kernel.h>
#include <linux/sys_call.h>
#include <linux/sched.h>
#include <linux/error.h>
#include <generated/autoconf.h>

static struct scheduler k_scheduler[CONFIG_CPU_NUM];

struct scheduler *get_scheduler_by_cpu_core_id(uint32_t core_id){
    if(core_id >= CONFIG_CPU_NUM) return NULL;
    return &k_scheduler[core_id];
}

static void scheduler_init( struct scheduler* uninit_scheduler,char* pool_name,int cpu_number)
{
    struct task_pool_types * task_pool = find_task_pool(pool_name);
    if(task_pool == NULL) {
        pr_err("can not find this task_pool:%s\n",pool_name);while (1){}
    }
    uninit_scheduler->s_task_pool = task_pool->alloc_task_pool(uninit_scheduler);
    if(IS_ERR(uninit_scheduler->s_task_pool)) {
        pr_err("can not init create scheduler\n");while (1){}
    }
    uninit_scheduler->scheduler_timer = 0;
    uninit_scheduler->magic  = scheduler_scheduler;
    uninit_scheduler->s_flag = SCHEDULER_BLOCKED;
    uninit_scheduler->current_task = NULL;
    uninit_scheduler->t_pop  = task_pool->t_op;
    uninit_scheduler->s_core = cpu_number;
}


extern size_t get_global_heap_size(void);
void default_task(void* argv){
    volatile int j  = 0;
    while (1){
        for(int i =0;i<50000000;i++){j++;}
        printk(KERN_DEBUG "system free memory size:%d kb core:%d\n\r",get_global_heap_size()/1024,__get_task_using_cpu_core());  
    }
}


#ifndef  CONFIG_SCHED_CPU0
#error "cpu core number is not zero"    
#endif
#ifndef CONFIG_SCHED_CPU1
#ifdef  CONFIG_SCHED_CPU0
static char* sched_config[] = {
    CONFIG_SCHED_CPU0
};
#endif 
#endif 
#ifndef CONFIG_SCHED_CPU2
#ifdef  CONFIG_SCHED_CPU1
static char* sched_config[] = {
    CONFIG_SCHED_CPU0,
    CONFIG_SCHED_CPU1,
};
#endif
#endif
#ifndef CONFIG_SCHED_CPU3
#ifdef CONFIG_SCHED_CPU2
static char* sched_config[] = {
    CONFIG_SCHED_CPU0,
    CONFIG_SCHED_CPU1,
    CONFIG_SCHED_CPU2,
};
#endif
#endif
#ifdef CONFIG_SCHED_CPU3
static char* sched_config[] = {
    CONFIG_SCHED_CPU0,
    CONFIG_SCHED_CPU1,
    CONFIG_SCHED_CPU2,
    CONFIG_SCHED_CPU3,
};
#endif
static void init_all_scheduler(void){
    for(int i =0 ;i < CONFIG_CPU_NUM ;i++){
        scheduler_init(get_scheduler_by_cpu_core_id(i),sched_config[i],i);
        task_run(default_task,512,10,1,"system_default_task",i,0);
    }
}





uint32_t Scheduler_Task(uint32_t context)
{
    struct scheduler * curren_scheduler = get_current_scheduler();

    if(curren_scheduler->s_flag == SCHEDULER_BLOCKED)
    return context;

    if(curren_scheduler->current_task != NULL)
    save_context(curren_scheduler->current_task, context);
    struct task_struct* t1 = curren_scheduler->current_task;
    curren_scheduler->current_task = curren_scheduler->t_pop->get_next_task(curren_scheduler);
    if(curren_scheduler->current_task!= NULL)
    return load_task(curren_scheduler->current_task);        
    else
    return context;
}




void __user_error_process(void){
    struct  task_struct* task = get_current_task();
    pr_err("__user_error_process: Task:%s get ERROR\n",task->task_name);
    task->state = TASK_BROKEN;
    __sched();
}


extern int check_haper_completeness(void);
extern void __destory_task(struct task_struct *t);
extern size_t get_global_heap_size(void);

void __kernel_error_process(void)
{
    uint32_t core_id = get_task_using_cpu_core();
    pr_err("cpu core: (%d) get ERROR\n",core_id);
    pr_err("checking memory completeness\n");
    uint32_t err = check_haper_completeness();
    if(err > 0){
        pr_err("memory ERROR:(%d)\n",err);
    }
    else {
        printk(KERN_DEBUG "system free memory size:%d kb\n\r",get_global_heap_size()/1024);  
    }
    struct scheduler* current_sched = &k_scheduler[core_id];
    if(current_sched->magic != scheduler_scheduler){
        pr_err("Can not get core: (%d) scheduler handler may be task is broken\n",core_id);
        goto block;
    }
    pr_err("get core: (%d) scheduler handler\n",core_id);
    struct  task_struct* task = current_sched->current_task;
    if(task == NULL){
        pr_err("scheduler is not working\n");
        i_sched();
        return;
    }
    if(task->magic != task_struct_magic){
        pr_err("Task controller is broken \n");
        goto block;
    }
    pr_err("__kernel_error_process: Task: %s get ERROR\n",task->task_name);
    if(current_sched->t_pop->check_task_completeness(current_sched) < 0){
        pr_err("Task controller has been broken \n");
        goto block;        
    }
    
    task->state = TASK_BROKEN;
    current_sched->current_task = NULL;
    current_sched->t_pop->remove_task(task,current_sched);
    __destory_task(task);
    i_sched();
    return;

    block:
    pr_err("System get Unfixable errors\n");
    while (1){}
}


void scheduler_start();

void __Task_manager_init();
void sched_init(){
    pr_info("sched: init all scheduler\n");
    __Task_manager_init();
    init_all_scheduler();    
}
core_initcall_sync(sched_init);



void current_sched_start()
{
    struct scheduler * curren_scheduler = get_current_scheduler();
    pr_info("get cpu(%d) scheruler\n",curren_scheduler->s_core);
    change_to_task_mode();
    curren_scheduler->s_flag = SCHEDULER_RUN;
    scheduler_start();
    while(1){};
}
postcore_initcall_sync(current_sched_start);