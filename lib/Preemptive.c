#include <linux/kernel.h> 
#include <linux/sched.h>
#include <linux/slab.h>
#include <generated/autoconf.h>


struct task_pool_Preemptive{
    struct task_struct *task_head;
    uint32_t tasknode_numbers;
};

static void remove_task_from_pool(struct task_struct* task,struct scheduler *sched);

static time64_t time = 0;
struct task_struct* get_next_task(struct task_struct* task){
    return task->next;
}

struct task_struct* get_useful_task(struct task_struct* head_task,struct scheduler *sched)
{
   struct task_struct* search_task = head_task;
   while (1)
   {
        if (search_task->state == TASK_DEAD) {
            struct task_struct *dead_task = search_task;
            struct task_struct *search_task = get_next_task(search_task);
            remove_task_from_pool(dead_task,sched);
            __destory_task(dead_task);
            if(search_task == NULL){ //保险作用
                search_task = head_task;
            }
        }
        if(search_task->state == TASK_READY){
            break;            
        }
        else if(search_task->state == TASK_WAITING){
            if(search_task->last_scheduler_time + search_task->block_time < time){
                search_task->state = TASK_READY;
                break;
            }
        }
        if(get_next_task(search_task) == NULL)
            break;
        search_task = get_next_task(search_task);
    }
 return search_task;
}

static struct task_struct* Preemptive_scheduling(struct scheduler *sched)
{   
    time++; 
    struct task_struct* next_task = get_useful_task(((struct task_pool_Preemptive*)sched->s_task_pool)->task_head,sched);    
    if(next_task != sched->current_task)
    {
        if( sched->current_task != NULL){
            sched->current_task->last_scheduler_time = time;
        }
        next_task->last_scheduler_time = time;
    }
    return next_task;
} 


static void show_all_task(struct task_struct* head)
{
    struct task_struct* t = head;
    if(t == NULL)return NULL;
    int i = 0;
    pr_info("--------------------------\n");
    while(1)
    {
        pr_info("task %d: %s \n",i,t->task_name);
        i++;
        t = t->next;
        if(t == NULL)
        break;
    }
    pr_info("--------------------------\n");
}


static int check_task_completeness(struct scheduler * sched)
{
     struct task_pool_Preemptive *pool = sched->s_task_pool;
     struct task_struct* t = pool->task_head;
     while(1)
     {
        if(t->magic != task_struct_magic)
        return -1;
        t = t->next;
        if(t == NULL)
        break;
     }
     return 0;
}




static struct task_struct* get_task_by_priority(struct task_pool_Preemptive* pool,uint32_t priority)
{
    struct task_struct* t = pool->task_head;
    if(t == NULL)return NULL;
    while(1)
    {
        if(t->priority <= priority || t == NULL)
            break;
        t = t->next;
    }
    return t;
}


static void add_a_task_node(struct task_struct* new,struct task_struct* task){
    new->next = task;
    new->priv = task->priv;
    task->priv->next = new;
    task->priv = new;
}
static void task_reset(struct task_struct* new){
    new->priv = NULL;
    new->next = NULL;
}
static int add_task_to_task_pool(struct task_struct* new ,struct scheduler *sched)
{
    struct task_pool_Preemptive *pool = sched->s_task_pool;
    task_reset(new);
    if(pool->task_head == NULL)
    {
        pool->task_head = new;      
    }
    else
    {
        struct task_struct *task = get_task_by_priority(pool,new->priority);
        if(task == NULL) 
        {
            add_a_task_node(new,pool->task_head);
            pool->task_head = new;
        }
        else
        {
            add_a_task_node(new,task);

        }        
    }
    if(pool->task_head->priv != NULL){
        pool->task_head = pool->task_head->priv;
    }

    //show_all_task(pool->task_head);
    return 0;
}




static void remove_task(struct task_struct* task)
{
    if(task->priv != NULL)
    task->priv->next = task->next;
    if(task->next != NULL)
    task->next->priv = task->priv;
}

static void remove_task_from_pool(struct task_struct* task,struct scheduler *sched)
{
    pr_info("remove task: %s\n",task->task_name);

    struct task_pool_Preemptive *pool = sched->s_task_pool;
    if(pool->task_head == task)
    {
        if(task->next == NULL) //如果这是最后一个任务，则不能删除
        return;            
        
        pool->task_head = task->next;
        remove_task(task);
    }
    else
    {
        remove_task(task);
    }
   
    if(sched->current_task == task){
        sched->current_task = NULL;
    }
    //show_all_task(pool->task_head);
}


static struct task_pool_Preemptive* alloc_new_pool(struct scheduler *sched){
    struct task_pool_Preemptive* new_task_pool = kmalloc(sizeof(struct task_pool_Preemptive),GFP_KERNEL);
    new_task_pool->task_head = NULL;
    new_task_pool->tasknode_numbers = 0;
    return new_task_pool;
}



static struct task_pool_operations task_op = {
    .get_next_task = Preemptive_scheduling,
    .add_task      = add_task_to_task_pool,
    .remove_task   = remove_task_from_pool,
    .check_task_completeness = check_task_completeness
};

static struct task_pool_types task_type = {
    .name = "Preemptive",
    .alloc_task_pool = alloc_new_pool,
    .t_op = &task_op,
};

static int __init init_Preemptive_fn(void){
    register_task_pool(&task_type);
}

#ifdef CONFIG_Preemptive
core_initcall(init_Preemptive_fn);
#endif // DEBUG




