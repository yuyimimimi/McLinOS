#include <linux/kernel.h> 
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/spinlock.h>

struct task_struct;
static spinlock_t Task_List_lock;
static struct list_head Task_List;
static struct list_head Death_task_List;

struct task_struct* init;




void __List_All_Tasks(void) {
    printk(KERN_INFO "[TaskManager] Listing all alive tasks:\n");
    struct task_struct *pos;
    list_for_each_entry(pos,&Task_List,task_node){
         printk(KERN_INFO "  - Task: %s (pid: %d)\n", pos->comm, pos->pid);
    }
    printk(KERN_INFO "[TaskManager] Listing all dead tasks:\n");
    list_for_each_entry(pos,&Death_task_List,task_node){
        printk(KERN_INFO "  - Task: %s (pid: %d)\n", pos->comm, pos->pid);
    }
}


void __Task_manager_init(){  
    spin_lock_init(&Task_List_lock);
    INIT_LIST_HEAD(&Task_List);
    INIT_LIST_HEAD(&Death_task_List);
}

void __save_Task(struct task_struct* new){ //注册任务时自动调用回调函数
    atomic_set(&new->use_count,1);
    new->parent = current;
    INIT_LIST_HEAD(&new->task_node);
    INIT_LIST_HEAD(&new->children);
    INIT_LIST_HEAD(&new->this_task_node);
    if(current != NULL)
    list_add(&new->this_task_node,&current->children);
    spin_lock(&Task_List_lock);
    list_add(&new->task_node,&Task_List);
    spin_unlock(&Task_List_lock);
    __List_All_Tasks();
}

void __Kill_Task(struct task_struct* new)//删除任务时自动调用回调函数
{
    spin_lock(&Task_List_lock);
    list_del(&new->task_node);
    list_add(&new->task_node,&Death_task_List);
    spin_unlock(&Task_List_lock); 
    __List_All_Tasks();
}

void __default_Task_return_function(void){
    struct task_struct* cutrrent_task = current;
    pr_info("task : %s has return\n" ,cutrrent_task->task_name);
    while (1){
        cutrrent_task->state = TASK_DEAD;
        sched();
    }
}

static void clean_task(struct task_struct* t)
{        
    // if(t->offset != NULL)
    //     kfree(t->offset);
    kfree(t);
}

pid_t sys_wait(int *status)
{
    struct task_struct* cutrrent_task = current;
    cutrrent_task->state = TASK_BLOCKED;       //进入阻塞状态，只能外部唤醒
    if(i_sched() != 0){                        //注意这是特权指令，只能在中断中调用sys_wait只能在syscall中调用
        cutrrent_task->state = TASK_RUNNING;
        return -1; 
    }
    cutrrent_task->script = WAITING_SON_TASK;
    struct task_struct *pos;
    list_for_each_entry(pos,&cutrrent_task->children,this_task_node){
        if(pos->state == TASK_DEAD){
            pid_t pid = pos->pid;
            clean_task(pos);
            return pid;        
        }
    }
    return -1;
}



