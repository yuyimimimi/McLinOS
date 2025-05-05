/* SPDX-License-Identifier: GPL-2.0 */
#ifndef BLK_INTERNAL_H
#define BLK_INTERNAL_H

#include <linux/spinlock_types.h>
#include <linux/blkdev.h>


extern struct dentry *blk_debugfs_root;

struct blk_flush_queue {
	spinlock_t		mq_flush_lock;
	unsigned int		flush_pending_idx:1;
	unsigned int		flush_running_idx:1;
	blk_status_t 		rq_status;
	unsigned long		flush_pending_since;
	struct list_head	flush_queue[2];
	unsigned long		flush_data_in_flight;
	struct request		*flush_rq;
};


#endif /* BLK_INTERNAL_H */