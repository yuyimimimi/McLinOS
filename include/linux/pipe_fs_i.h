/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PIPE_FS_I_H
#define _LINUX_PIPE_FS_I_H

#define PIPE_DEF_BUFFERS	16

#define PIPE_BUF_FLAG_LRU	0x01	/* page is on the LRU */
#define PIPE_BUF_FLAG_ATOMIC	0x02	/* was atomically mapped */
#define PIPE_BUF_FLAG_GIFT	0x04	/* page is a gift */
#define PIPE_BUF_FLAG_PACKET	0x08	/* read() as a packet */
#define PIPE_BUF_FLAG_CAN_MERGE	0x10	/* can merge buffers */
#define PIPE_BUF_FLAG_WHOLE	0x20	/* read() must return entire buffer or error */
#ifdef CONFIG_WATCH_QUEUE
#define PIPE_BUF_FLAG_LOSS	0x40	/* Message loss happened after this buffer */
#endif
#include <generated/autoconf.h>

#define PIPE_BUFFER_SIZE CONFIG_PIPE_FILE_SYSTEM_NODE_BUFFER_SIZE

struct pipe_buffer {
    uint8_t buffer[PIPE_BUFFER_SIZE];    // 环形缓冲区指针
    size_t size;        // 缓冲区总容量
    size_t head;        // 写指针
    size_t tail;        // 读指针
	spinlock_t		lock;
};

struct pipefs_inode {
    int                     magic;
    uint32_t                i_mode;
    uint32_t                major;
    atomic_t                dentry_count;          //引用计数
    struct file_operations *i_fop;                
    struct list_head        list_node;             // 挂在superblock的inode链表
    struct list_head        dentry_list_head;      // 如果这是目录项，这里存储inode下的所有dentry链表头
    spinlock_t              lock;
    struct pipefs_superblock *sb;
    struct pipe_buffer      *buf;
};

#endif
