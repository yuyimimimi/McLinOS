#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>

static LIST_HEAD(task_pool_list);

int register_task_pool(struct task_pool_types *new_pool){
    struct task_pool_types *entry;
    list_for_each_entry(entry, &task_pool_list, node) {
        if (strcmp(entry->name, new_pool->name) == 0) {
            pr_warn("task pool %s already registered\n", new_pool->name);
            return -EEXIST;
        }
    }
    list_add_tail(&new_pool->node, &task_pool_list);
    pr_info("Registered task pool: %s\n", new_pool->name);
    return 0;
}

struct task_pool_types *find_task_pool(const char *name){
    struct task_pool_types *entry;
    list_for_each_entry(entry, &task_pool_list, node) {
        if (strcmp(entry->name, name) == 0)
            return entry;
    }
    return NULL; 
}

int unregister_task_pool(const char *name){
    struct task_pool_types *entry, *tmp;
    list_for_each_entry_safe(entry, tmp, &task_pool_list, node) {
        if (strcmp(entry->name, name) == 0) {
            list_del(&entry->node);
            pr_info("Unregistered task pool: %s\n", name);
            kfree(entry->name);
            kfree(entry);
            return 0;
        }
    }
    return -ENOENT;
}
