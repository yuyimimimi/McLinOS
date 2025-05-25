#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/string.h>

extern struct export_node_struct __export_table_start[];
extern struct export_node_struct __export_table_end[];

void* find_table_by_fnname(char *name)
{
    if(name == NULL) return 0;
    for (struct export_node_struct *index =__export_table_start; index < __export_table_end; index++)
    {
       if(index->export_node_name != NULL )
       {
           int i = strcmp(index->export_node_name,name);        
           if(i == 0){
             return index->export_node_fn_address;
           }
        }
    }
    return NULL;
}


