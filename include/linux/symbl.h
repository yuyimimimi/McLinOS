#ifndef __SYMBLE_H_
#define __SYMBLE_H_
#include <linux/types.h>
void* find_table_by_fnname(char *name);
void* find_symbol_by_fnname_from_bootloader(char *name);
#endif