/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H
/*
 * Linux wait queue related types and methods
 */
#include <linux/list.h>
#include <linux/stddef.h>
#include <linux/spinlock.h>


struct wait_queue_head {
	spinlock_t		lock;
	struct list_head	head;
};

typedef struct wait_queue_head wait_queue_head_t;


#endif /* _LINUX_WAIT_H */
