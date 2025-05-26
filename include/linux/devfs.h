#ifndef DEVFS_H_
#define DEVFS_H_

#include <linux/fs.h>

extern int devfs_mount_device(struct inode *inode,unsigned int major,struct file_operations *fop);
extern int devfs_mount_blk_device(struct inode *inode,struct block_device* bdev,uint32_t major);
extern struct block_device* devfs_get_blk_dev(struct inode *inode);

#endif
