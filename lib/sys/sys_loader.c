#include <linux/export.h>

int loader_user_app(char* file_path,void* argv);

int sys_execve(const char __user *file_path, void __user *argv,char *const envp[]){
    return loader_user_app(file_path,argv);
}
EXPORT_SYMBOL(sys_execve);

