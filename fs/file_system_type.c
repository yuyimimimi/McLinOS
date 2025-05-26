#include<linux/kernel.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/string.h>


struct file_system_type *fs_t = NULL;

int register_filesystem(struct file_system_type *ft)
{
    if(ft == NULL) return -1;
    pr_info("register file system: %s\n",ft->name);
    struct file_system_type **p;
    for (p = &fs_t; *p; p = &(*p)->next) {
        if (strcmp((*p)->name, ft->name) == 0)
            return -EEXIST; 
    }
    ft->next = fs_t;
    fs_t = ft;
    return 0;
}
EXPORT_SYMBOL(register_filesystem);

int unregister_filesystem(struct file_system_type *ft)
{
    struct file_system_type **p;
    for (p = &fs_t; *p; p = &(*p)->next) {
        if (*p == ft) {
            *p = ft->next; 
            return 0;
        }
    }
    return -EINVAL;
}
EXPORT_SYMBOL(unregister_filesystem);

struct file_system_type *lookup_fs_type(const char *name)
{
    struct file_system_type *p;
    
    for (p = fs_t; p; p = p->next) {
        if (strcmp(p->name, name) == 0)
            return p;
    }
    return NULL;
}
EXPORT_SYMBOL(lookup_fs_type);
