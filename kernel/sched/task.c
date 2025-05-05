#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/error.h>
#include <linux/string.h>

static uint32_t id_count = 9;
struct task_struct* __new_task_create(
            int (*entry)(void*), 
            int stack_size,
            void *argv,
            int priority,
            char *name
            )
{
    if(entry == NULL || name == NULL){
        pr_info(KERN_INFO "can not create t\n");
        return -1;
    }
    stack_size += 64;
    stack_size += sizeof(struct task_struct);
    stack_size = (stack_size + 127) & ~127; 
    struct task_struct *new_task = kmalloc(stack_size, GFP_NOWAIT);
    if (new_task == NULL){
        printk("can not alloc memory1\n\r");
        return -ENOMEM;
    } 
    if(priority == 0)
        priority = defauld_thread_priority;
    
    new_task->stack_ptr = (void*)new_task + sizeof(struct task_struct);
    new_task->magic  = task_struct_magic;
    new_task->id     = id_count++;
    new_task->entry  = entry;
    new_task->arg    = argv;
    new_task->priority = priority;
    new_task->stack_Top = (void*)new_task + stack_size;

    if(strlen(name) < task_name_max_len){
        strcpy(new_task->task_name,name);
    }
    else{
        memcpy(new_task->task_name,name,task_name_max_len -1);
        new_task->task_name[task_name_max_len -1] = '\0';
    }
    init_task_context(new_task);
    new_task->state = TASK_READY;
    return new_task;
}

void __destory_task(struct task_struct *t) 
{
    if(t == NULL) return;
    kfree(t);
}


int __register_task(struct task_struct* new_task ,struct scheduler* scheduler){
   return scheduler->t_pop->add_task(new_task,scheduler);
}

void __default_Task_return_function(void){
    struct task_struct* cutrrent_task = get_current_task();
    pr_info("task :%s has return\n" ,cutrrent_task->task_name);
    cutrrent_task->state = TASK_DEAD;
    while (1)
    {
        __delay(10);
        cutrrent_task->state = TASK_DEAD;
    }
} 


struct task_struct* task_run(       
int (*entry)(void*), 
int stack_size,
void *argv,
int priority,
char *name,
uint32_t core_id)
{
    struct scheduler * schedule = 
    get_scheduler_by_cpu_core_id(core_id);
    if(schedule == NULL){
        pr_err("cpu number err");
        return NULL;
    }
    struct task_struct* task =
    __new_task_create(entry,stack_size,argv,priority,name);
    if(IS_ERR(task)){
        return NULL;   
    }
    if( __register_task(task,schedule) < 0)
    {
        __destory_task(task);
        return NULL;
    }
    return task;
}

