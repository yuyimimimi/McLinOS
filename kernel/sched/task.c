#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/error.h>
#include <linux/string.h>
#include <linux/atomic.h>

static uint32_t id_count = 1;
struct task_struct* __new_task_create(
            int (*entry)(void*), 
            int stack_size,
            void *argv,
            int priority,
            char *name,
            uint32_t offset
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
        pr_info("can not alloc memory: need %d kb\n\r",stack_size/1024);
        return -ENOMEM;
    }  
    if(priority == 0)
        priority = defauld_thread_priority;   
    new_task->stack_ptr = (void*)new_task + sizeof(struct task_struct);
    new_task->magic  = task_struct_magic;
    new_task->id     = id_count++;
    new_task->pid    = new_task->id; 
    new_task->entry  = entry;
    new_task->arg    = argv;
    new_task->priority = priority;
    new_task->stack_Top = (void*)new_task + stack_size;
    new_task->offset  = offset;
    new_task->comm    = new_task->task_name;

    if(strlen(name) < task_name_max_len){   
        strcpy(new_task->task_name,name); 
    }
    else{  
        memcpy(new_task->task_name,name,task_name_max_len -1);
        new_task->task_name[task_name_max_len -1] = '\0';
    }
    init_task_context(new_task,offset);
    new_task->state = TASK_READY;

    return new_task;
}



void __Kill_Task(struct task_struct* new);
void __destory_task(struct task_struct *t) 
{
    if(t == NULL) return;
    __Kill_Task(t);
}


void __save_Task(struct task_struct* new);
int __register_task(struct task_struct* new_task ,struct scheduler* scheduler)
{
   __save_Task(new_task);
   return scheduler->t_pop->add_task(new_task,scheduler);
}


void exit(){
    __default_Task_return_function();
}


struct task_struct* task_run(       
int (*entry)(void*), 
int stack_size,
void *argv,
int priority,
char *name,
uint32_t core_id,
uint32_t offset
)
{
    struct scheduler * schedule = 
    get_scheduler_by_cpu_core_id(core_id);
    if(schedule == NULL){
        pr_err("cpu number err\n");
        return NULL;
    }
    struct task_struct* task =
    __new_task_create(entry,stack_size,argv,priority,name,offset);
    if(IS_ERR(task)){
        return task;   
    }
    if( __register_task(task,schedule) < 0)
    {
        __destory_task(task);
        return NULL;
    }
    pr_info("sched: task : %s(%d) has create\n" ,name,task->id);
    return task;
}


