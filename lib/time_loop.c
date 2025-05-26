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

static struct task_struct* time_loop(struct scheduler *sched)
{   
    struct task_struct* c_task = sched->current_task;
    if(c_task == NULL){
        return ((struct task_pool_1*)sched->s_task_pool)->head_node->task;
    }
    struct task_node* node = (struct task_node*)c_task->t_node;
    struct task_struct* next_task =  node->next->task;
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
    if(pool->head_node == NULL)
    {
        pool->head_node = new_node;
        new_node->next  = new_node;
        new_node->priv  = new_node;
    }
    else
    {
        struct task_node * n = pool->head_node ;
        while (n->priority > new_node->priority){
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
    run_scheduler(sched);
    return 0;
}

static void remove_task_from_pool(struct task_struct* task,struct scheduler *sched)
{
    struct task_node* node = (struct task_node*)task->t_node;

    block_scheduler(sched);
    node->priv->next = node->next;
    node->next->priv = node->priv;
    run_scheduler(sched);
    
    kfree(node);
    task->t_node = NULL;
}

static struct task_pool_1* alloc_new_pool(struct scheduler *sched){
    struct task_pool_1* new_task_pool = kmalloc(sizeof(struct task_pool_1),GFP_KERNEL);
    new_task_pool->head_node = NULL;
    new_task_pool->tasknode_numbers = 0;
    return new_task_pool;
}

static struct task_pool_operations task_op = {
    .get_next_task = time_loop,
    .add_task      = add_task_to_task_pool,
    .remove_task   = remove_task_from_pool,
};

static struct task_pool_types task_type = {
    .name = "time_loop",
    .alloc_task_pool = alloc_new_pool,
    .t_op = &task_op,
};

static int __init init_time_loop_fn(){
    register_task_pool(&task_type);
}

#ifdef CONFIG_time_loop
core_initcall(init_time_loop_fn);
#endif 
