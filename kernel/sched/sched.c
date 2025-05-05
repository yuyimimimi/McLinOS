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

static void scheduler_init( struct scheduler* uninit_scheduler,char* pool_name)
{
    struct task_pool_types * task_pool = find_task_pool(pool_name);
    if(task_pool == NULL) {
        pr_err("can not find this task_pool:%s\n",pool_name);while (1){}
    }
    uninit_scheduler->s_task_pool = task_pool->alloc_task_pool(uninit_scheduler);
    if(IS_ERR(uninit_scheduler->s_task_pool)) {
        pr_err("can not init create scheduler\n");while (1){}
    }
    uninit_scheduler->s_flag = SCHEDULER_RUN;
    uninit_scheduler->current_task = NULL;
    uninit_scheduler->t_pop  = task_pool->t_op;
}

void default_task(void* argv){
    volatile int j  = 0;
    while (1){
        for(int i =0;i<120000000;i++){j++;}
        printk("default_task test\n");
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
        scheduler_init(get_scheduler_by_cpu_core_id(i),sched_config[i]);
        task_run(default_task,512,10,1,"system_default_task",i);
    }
}

uint32_t Scheduler_Task(uint32_t context)
{
    struct scheduler * curren_scheduler = get_current_scheduler();
    if(curren_scheduler->s_flag == SCHEDULER_BLOCKED )
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

void scheduler_start();

void sched_init(){
    pr_info("init all scheduler\n");
    init_all_scheduler();    
}
core_initcall_sync(sched_init);

void sched_start(){
    change_to_task_mode();
    scheduler_start();
    user_system_call(SC_SCHEDULER,NULL,NULL,NULL,NULL,NULL,NULL);
}
postcore_initcall_sync(sched_start);
