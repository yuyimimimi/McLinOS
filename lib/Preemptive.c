#include <linux/kernel.h> 
#include <linux/sched.h>
#include <linux/slab.h>
#include <generated/autoconf.h>

struct task_node{
    struct task_node *next;
    struct task_node *priv;
    struct task_struct *task;
    int priority;
};

struct task_pool_1{
    struct task_node *head_node;
    uint32_t tasknode_numbers;
};





static void remove_task_from_pool(struct task_struct* task,struct scheduler *sched);



static time64_t time = 0;
struct task_struct* get_next_task(struct task_struct* task){
    return ((struct task_node*)task->t_node)->next->task;
}

struct task_struct* get_useful_task(struct task_pool_1 *task_pool,struct scheduler *sched)
{
    struct task_struct* head_task = task_pool->head_node->task;
    struct task_struct* search_task = head_task;
    while (1)
    {
        if (search_task->state == TASK_DEAD) 
        {
            struct task_struct *dead_task = search_task;
            struct task_struct *search_task = get_next_task(search_task);
            remove_task_from_pool(dead_task, sched);
            __destory_task(dead_task);
        }
        if(search_task->state == TASK_READY)
            break;
        else if(search_task->state == TASK_WAITING){
            if(search_task->last_scheduler_time + search_task->block_time
               < time){
                search_task->state = TASK_READY;
                break;
            }
        }
        if(get_next_task(search_task) == head_task)
            break;
        search_task = get_next_task(search_task);
    }
    return search_task;
}

static struct task_struct* Preemptive_scheduling(struct scheduler *sched)
{   
    time++;    
    struct task_struct* next_task = get_useful_task(sched->s_task_pool,sched);
    if(next_task != sched->current_task)
    {
        if( sched->current_task != NULL){
            sched->current_task->last_scheduler_time = time;
        }
        next_task->last_scheduler_time = time;
    }
    return next_task;
} 

static int add_task_to_task_pool(struct task_struct* task ,struct scheduler *sched)
{
    struct task_pool_1 *pool = sched->s_task_pool;
    struct task_node* new_node = kmalloc(sizeof(struct task_node),GFP_KERNEL);
    if (new_node == NULL){
        return -1;
    }
    new_node->task     = task;
    new_node->priority = task->priority;
    task->t_node       = new_node;

    block_scheduler(sched);
    if(pool->head_node == NULL){
        pool->head_node = new_node;
        new_node->next  = new_node;
        new_node->priv  = new_node;
    }
    else
    {
        struct task_node * n = pool->head_node;
        while (n->priority > new_node->priority)
        {
            n = n->next;
            if(n == pool->head_node){
            break;
            }
        }
        new_node->next = n;
        new_node->priv = n->priv;
        n->priv->next  = new_node;
        n->priv        = new_node;
    }
    if (new_node->priority > pool->head_node->priority) {
        pool->head_node = new_node;
    }
    run_scheduler(sched);
    return 0;
}

static void remove_task_from_pool(struct task_struct* task,struct scheduler *sched)
{
    struct task_node* node = (struct task_node*)task->t_node;
    struct task_pool_1 *pool = sched->s_task_pool;
    if (pool->head_node == node) {
            pool->head_node = node->next;
    }
    if(sched->current_task == task){
        sched->current_task = NULL;
    }
    node->priv->next = node->next;
    node->next->priv = node->priv;
    kfree(node);
}


static struct task_pool_1* alloc_new_pool(struct scheduler *sched){
    struct task_pool_1* new_task_pool = kmalloc(sizeof(struct task_pool_1),GFP_KERNEL);
    new_task_pool->head_node = NULL;
    new_task_pool->tasknode_numbers = 0;
    return new_task_pool;
}

static struct task_pool_operations task_op = {
    .get_next_task = Preemptive_scheduling,
    .add_task      = add_task_to_task_pool,
    .remove_task   = remove_task_from_pool,
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




