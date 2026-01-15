#include <linux/sched.h>
#include <linux/export.h>

struct task_struct *loader_user_app(char* file_path,void* argv);

struct task_struct * sys_execve(const char __user *file_path, void __user *argv,char *const envp[]){
    return loader_user_app(file_path,argv);
}
EXPORT_SYMBOL(sys_execve);

