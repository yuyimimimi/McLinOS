// SPDX-License-Identifier: GPL-2.0

#include <linux/compiler.h>

#ifndef __CHECKER__

#define  __weak __attribute__((weak))

// Minimal definition for struct ftrace_likely_data
struct ftrace_likely_data {
    void *func;
    const char *file;
    int line;
    unsigned int miss_hit[2];
};

void __weak ftrace_likely_update(struct ftrace_likely_data *f, int val,
                                 int expect, int is_constant)
{
    // Minimal implementation: do nothing
}

// Weak symbols for KCSAN functions, so they can be overridden by KCSAN
void  __weak  __kcsan_disable_current(void)
{
    // Minimal implementation: do nothing
}

void __weak  __kcsan_enable_current(void)
{
    // Minimal implementation: do nothing
}

// Minimal implementation for mb() if not defined elsewhere
#ifndef mb
#define mb() __asm__ __volatile__("": : :"memory")
#endif


void __weak  annotate_unreachable(void)
{
    // Minimal implementation: do nothing
}





#endif // __CHECKER__