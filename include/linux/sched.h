#ifndef __LINUX_SCHED_H_
#define __LINUX_SCHED_H_


#include <linux/time.h>
#include <linux/list.h>
#include <asm/sched.h>


#define defauld_thread_priority      8
#define defauld_Task_watch_dog_tick  1000 
#define task_struct_magic            123456
#define task_name_max_len            64
enum task_state {
    TASK_CREATE  = 1,     //创建
    TASK_READY   = 2,     //就绪
    TASK_WAITING = 3,     //等待
    TASK_BLOCKED = 4,     //完全阻塞
    TASK_RUNNING = 5,     //运行
    TASK_DEAD    = 6,     //死亡(不会被立即清理)
    TASK_BROKEN  = 7,     //任务内部发生致命错误
};
enum script {
    WAITING_SON_TASK = 0
};

atomic_t;
struct task_struct 
{
    uint32_t           magic;
    uint32_t           id;               //任务ID
    uint32_t           pid;   
    
    char               task_name[task_name_max_len];         
    char*              comm;
    
    TaskContext        context;         //任务上下文
    uint32_t           *stack_ptr;       //任务栈指针
    uint32_t           *stack_Top;       //任务栈底指针
    void (*entry)(void*);                //任务入口函数
    void *arg;                           //任务入口函数参数
    enum task_state state;               //任务状态
    struct scheduler *task_scheduler;    //任务调度器
    int priority;
    void* t_node;

    time64_t last_scheduler_time;        //上一次调度状态变更时间
    timer_t  block_time;                 //阻塞时间

    struct task_struct *priv;
    struct task_struct *next;
    
        enum script script;         //用于标识执行模式
        struct list_head children;
        struct list_head task_node; //用于添加到全局链表或死亡链表
        struct list_head this_task_node; //用于添加到父进程链表中
        atomic_t use_count;
        struct task_struct *parent;
    

    uint32_t offset;
};

struct task_pool_operations {
    struct task_struct* (*get_next_task)(struct scheduler *);
    int  (*add_task)(struct task_struct* ,struct scheduler * );
    void (*remove_task)(struct task_struct* ,struct scheduler *);
    int (*check_task_completeness)(struct scheduler *);
};


struct task_pool_types{
    struct list_head node;
    char *name;
    struct task_pool_operations *t_op;
    void*(*alloc_task_pool)(struct scheduler *);
};
enum scheduler_block_flag {
    SCHEDULER_BLOCKED = 0, 
    SCHEDULER_RUN     = 1,
};

#define scheduler_scheduler  0x123456
struct scheduler
{
    uint64_t            scheduler_timer;
    uint32_t                      magic;
    void                   *s_task_pool;
    uint32_t                     s_core;
    struct task_pool_operations*  t_pop;
    enum scheduler_block_flag    s_flag;
    struct task_struct*    current_task;
};

#include <asm/context.h>

void exit();
void sched(void);
int i_sched(void); //只能在特权指令下使用
void block_scheduler(struct scheduler *s);
void run_scheduler(struct scheduler *s);
void start_all_scheduler(void);
void stop_all_scheduler(void);
void __delay(uint32_t time);


struct scheduler *get_current_scheduler(void);
struct task_struct* get_current_task(void);

int register_task_pool(struct task_pool_types *new_pool);
struct task_pool_types *find_task_pool(const char *name);
int unregister_task_pool(const char *name);

struct scheduler *get_scheduler_by_cpu_core_id(uint32_t core_id);
static void default_init_all_scheduler(char* name);

struct task_struct* task_run(       
    int (*entry)(void*), 
    int stack_size,
    void *argv,
    int priority,
    char *name,
    uint32_t core_id,
    uint32_t offset
);

#endif
