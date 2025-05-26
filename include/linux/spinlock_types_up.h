#ifndef __LINUX_SPINLOCK_TYPES_UP_H
#define __LINUX_SPINLOCK_TYPES_UP_H

#ifndef __LINUX_SPINLOCK_TYPES_RAW_H
# error "Please do not include this file directly."
#endif

/*
 * include/linux/spinlock_types_up.h - spinlock type definitions for UP
 *
 * portions Copyright 2005, Red Hat, Inc., Ingo Molnar
 * Released under the General Public License (GPL).
 */

#include <asm/spinlock.h>


typedef struct {
	/* no debug version on UP */
} arch_rwlock_t;

#define __ARCH_RW_LOCK_UNLOCKED { }

#endif /* __LINUX_SPINLOCK_TYPES_UP_H */
