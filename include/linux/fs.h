/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FS_H
#define _LINUX_FS_H

#include <linux/types.h>
#include <linux/errseq.h> 
#include <linux/spinlock.h> 
#include <linux/list.h>
#include <linux/rbtree_types.h>  
#include <linux/uuid.h>  
#include <linux/uidgid_types.h>   
#include <linux/projid.h>       
#include <linux/time64.h> 
#include <linux/mnt_idmapping.h>  
#include <linux/mutex.h>         
#include <linux/uio.h>              
#include <linux/migrate_mode.h>  
#include <linux/raid/pq.h> 
#include <linux/lockdep_types.h>
#include <linux/wait.h>				
#include <linux/pipe_fs_i.h>
#include <linux/fcntl.h>
#include <linux/xarray.h>
#include <linux/blkdev.h>
#include <linux/dcache.h>
#include <linux/mm_type.h>
#include <linux/statfs.h>
#include <linux/stat.h>
#include <linux/path.h>


struct dentry;
struct file;
struct inode;
struct kiocb;
struct file_operations;
struct inode_operations  ;
struct dentry_operations ;
struct writeback_control ;
struct address_space;
struct seq_file ;
struct shrink_control;
struct file_system_type;


#define MAY_EXEC		0x00000001
#define MAY_WRITE		0x00000002
#define MAY_READ		0x00000004
#define MAY_APPEND		0x00000008
#define MAY_ACCESS		0x00000010
#define MAY_OPEN		0x00000020
#define MAY_CHDIR		0x00000040
/* called from RCU mode, don't block */
#define MAY_NOT_BLOCK		0x00000080

/*
 * flags in file.f_mode.  Note that FMODE_READ and FMODE_WRITE must correspond
 * to O_WRONLY and O_RDWR via the strange trick in do_dentry_open()
 */

/* file is open for reading */
#define FMODE_READ		((__force fmode_t)(1 << 0))
/* file is open for writing */
#define FMODE_WRITE		((__force fmode_t)(1 << 1))
/* file is seekable */
#define FMODE_LSEEK		((__force fmode_t)(1 << 2))
/* file can be accessed using pread */
#define FMODE_PREAD		((__force fmode_t)(1 << 3))
/* file can be accessed using pwrite */
#define FMODE_PWRITE		((__force fmode_t)(1 << 4))
/* File is opened for execution with sys_execve / sys_uselib */
#define FMODE_EXEC		((__force fmode_t)(1 << 5))
/* File writes are restricted (block device specific) */
#define FMODE_WRITE_RESTRICTED	((__force fmode_t)(1 << 6))
/* File supports atomic writes */
#define FMODE_CAN_ATOMIC_WRITE	((__force fmode_t)(1 << 7))

/* FMODE_* bit 8 */

/* 32bit hashes as llseek() offset (for directories) */
#define FMODE_32BITHASH         ((__force fmode_t)(1 << 9))
/* 64bit hashes as llseek() offset (for directories) */
#define FMODE_64BITHASH         ((__force fmode_t)(1 << 10))

/*
 * Don't update ctime and mtime.
 *
 * Currently a special hack for the XFS open_by_handle ioctl, but we'll
 * hopefully graduate it to a proper O_CMTIME flag supported by open(2) soon.
 */
#define FMODE_NOCMTIME		((__force fmode_t)(1 << 11))

/* Expect random access pattern */
#define FMODE_RANDOM		((__force fmode_t)(1 << 12))

/* FMODE_* bit 13 */

/* File is opened with O_PATH; almost nothing can be done with it */
#define FMODE_PATH		((__force fmode_t)(1 << 14))

/* File needs atomic accesses to f_pos */
#define FMODE_ATOMIC_POS	((__force fmode_t)(1 << 15))
/* Write access to underlying fs */
#define FMODE_WRITER		((__force fmode_t)(1 << 16))
/* Has read method(s) */
#define FMODE_CAN_READ          ((__force fmode_t)(1 << 17))
/* Has write method(s) */
#define FMODE_CAN_WRITE         ((__force fmode_t)(1 << 18))

#define FMODE_OPENED		((__force fmode_t)(1 << 19))
#define FMODE_CREATED		((__force fmode_t)(1 << 20))

/* File is stream-like */
#define FMODE_STREAM		((__force fmode_t)(1 << 21))

/* File supports DIRECT IO */
#define	FMODE_CAN_ODIRECT	((__force fmode_t)(1 << 22))

#define	FMODE_NOREUSE		((__force fmode_t)(1 << 23))

/* FMODE_* bit 24 */

/* File is embedded in backing_file object */
#define FMODE_BACKING		((__force fmode_t)(1 << 25))

/* File was opened by fanotify and shouldn't generate fanotify events */
#define FMODE_NONOTIFY		((__force fmode_t)(1 << 26))

/* File is capable of returning -EAGAIN if I/O will block */
#define FMODE_NOWAIT		((__force fmode_t)(1 << 27))

/* File represents mount that needs unmounting */
#define FMODE_NEED_UNMOUNT	((__force fmode_t)(1 << 28))

/* File does not contribute to nr_files count */
#define FMODE_NOACCOUNT		((__force fmode_t)(1 << 29))

/*
 * Attribute flags.  These should be or-ed together to figure out what
 * has been changed!
 */
#define ATTR_MODE	(1 << 0)
#define ATTR_UID	(1 << 1)
#define ATTR_GID	(1 << 2)
#define ATTR_SIZE	(1 << 3)
#define ATTR_ATIME	(1 << 4)
#define ATTR_MTIME	(1 << 5)
#define ATTR_CTIME	(1 << 6)
#define ATTR_ATIME_SET	(1 << 7)
#define ATTR_MTIME_SET	(1 << 8)
#define ATTR_FORCE	(1 << 9) /* Not a change, but a change it */
#define ATTR_KILL_SUID	(1 << 11)
#define ATTR_KILL_SGID	(1 << 12)
#define ATTR_FILE	(1 << 13)
#define ATTR_KILL_PRIV	(1 << 14)
#define ATTR_OPEN	(1 << 15) /* Truncating from open(O_TRUNC) */
#define ATTR_TIMES_SET	(1 << 16)
#define ATTR_TOUCH	(1 << 17)
#define ATTR_DELEG	(1 << 18) /* Delegated attrs. Don't break write delegations */

/*
 * Whiteout is represented by a char device.  The following constants define the
 * mode and device number to use.
 */
#define WHITEOUT_MODE 0
#define WHITEOUT_DEV 0

struct buffer_head {
	char * b_data;			/* pointer to data block (1024 bytes) 数据块 */
	unsigned long b_blocknr;	/* block number 块号 */
	unsigned short b_dev;		/* device (0 = free) 数据源的设备号 */
	unsigned char b_uptodate;   // 更新标志，表示数据是否已经更新
	unsigned char b_dirt;		/* 0-clean,1-dirty */
	unsigned char b_count;		/* users using this block */  //使用的用户数, reference count? 
	unsigned char b_lock;		/* 0 - ok, 1 -locked */
	struct mutex b_wait;
	struct buffer_head * b_prev;		// 前一块（这四个指针用于缓冲区的管理）
	struct buffer_head * b_next;		// 下一块
	struct buffer_head * b_prev_free;	// 前一空闲块
	struct buffer_head * b_next_free;	// 下一空闲块
};

/********************************************************
 * 														*
 * 			   		     iattr  						*
 *														* 
*********************************************************/


struct iattr {
	unsigned int	ia_valid;
	umode_t		ia_mode;
	union {
		kuid_t		ia_uid;
		vfsuid_t	ia_vfsuid;
	};
	union {
		kgid_t		ia_gid;
		vfsgid_t	ia_vfsgid;
	};
	loff_t		ia_size;
	struct timespec64 ia_atime;
	struct timespec64 ia_mtime;
	struct timespec64 ia_ctime;
	struct file	*ia_file;
};

/********************************************************
 * 														*
 * 			     address_space  						*
 *														* 
*********************************************************/
struct file_ra_state {
	pgoff_t start;
	unsigned int size;
	unsigned int async_size;
	unsigned int ra_pages;
	unsigned int mmap_miss;
	loff_t prev_pos;
};

struct iov_iter {
	u8 iter_type;
	bool nofault;
	bool data_source;
	size_t iov_offset;
	/*
	 * Hack alert: overlay ubuf_iovec with iovec + count, so
	 * that the members resolve correctly regardless of the type
	 * of iterator used. This means that you can use:
	 *
	 * &iter->__ubuf_iovec or iter->__iov
	 *
	 * interchangably for the user_backed cases, hence simplifying
	 * some of the cases that need to deal with both.
	 */
	union {
		/*
		 * This really should be a const, but we cannot do that without
		 * also modifying any of the zero-filling iter init functions.
		 * Leave it non-const for now, but it should be treated as such.
		 */
		struct iovec __ubuf_iovec;
		struct {
			union {
				/* use iter_iov() to get the current vec */
				const struct iovec *__iov;
				const struct kvec *kvec;
				const struct bio_vec *bvec;
				const struct folio_queue *folioq;
				struct xarray *xarray;
				void __user *ubuf;
			};
			size_t count;
		};
	};
	union {
		unsigned long nr_segs;
		u8 folioq_slot;
		loff_t xarray_start;
	};
};

struct address_space_operations {
	int (*writepage)(struct page *page, struct writeback_control *wbc);
	int (*read_folio)(struct file *, struct folio *);

	/* Write back some dirty pages from this mapping. */
	int (*writepages)(struct address_space *, struct writeback_control *);

	/* Mark a folio dirty.  Return true if this dirtied it */
	bool (*dirty_folio)(struct address_space *, struct folio *);

	void (*readahead)(struct readahead_control *);

	int (*write_begin)(struct file *, struct address_space *mapping,
				loff_t pos, unsigned len,
				struct folio **foliop, void **fsdata);
	int (*write_end)(struct file *, struct address_space *mapping,
				loff_t pos, unsigned len, unsigned copied,
				struct folio *folio, void *fsdata);

	/* Unfortunately this kludge is needed for FIBMAP. Don't use it */
	sector_t (*bmap)(struct address_space *, sector_t);
	void (*invalidate_folio) (struct folio *, size_t offset, size_t len);
	bool (*release_folio)(struct folio *, gfp_t);
	void (*free_folio)(struct folio *folio);
	ssize_t (*direct_IO)(struct kiocb *, struct iov_iter *iter);
	/*
	 * migrate the contents of a folio to the specified target. If
	 * migrate_mode is MIGRATE_ASYNC, it must not block.
	 */
	int (*migrate_folio)(struct address_space *, struct folio *dst,
			struct folio *src, enum migrate_mode);
	int (*launder_folio)(struct folio *);
	bool (*is_partially_uptodate) (struct folio *, size_t from,
			size_t count);
	void (*is_dirty_writeback) (struct folio *, bool *dirty, bool *wb);
	int (*error_remove_folio)(struct address_space *, struct folio *);

	/* swapfile support */
	// int (*swap_activate)(struct swap_info_struct *sis, struct file *file,
	// 			sector_t *span);
	// void (*swap_deactivate)(struct file *file);
	// int (*swap_rw)(struct kiocb *iocb, struct iov_iter *iter);
};

 struct address_space {
	struct inode		*host;
	struct xarray		i_pages;
	gfp_t			gfp_mask;
	atomic_t		i_mmap_writable;
#ifdef CONFIG_READ_ONLY_THP_FOR_FS
	/* number of thp, only for non-shmem files */
	atomic_t		nr_thps;
#endif
	struct rb_root_cached	i_mmap;
	unsigned long		nrpages;
	pgoff_t			writeback_index;
	const struct address_space_operations *a_ops;
	unsigned long		flags;
	errseq_t		wb_err;
	spinlock_t		i_private_lock;
	struct list_head	i_private_list;
	void *			i_private_data;
} __attribute__((aligned(sizeof(long)))) __randomize_layout;

/********************************************************
 * 														*
 * 			   		  file struct						*
 *														* 
*********************************************************/

struct file {
	struct mutex					f_ref;
	struct mutex					f_lock;
	fmode_t							f_mode;
	const struct file_operations	*f_op;                            
	//	struct address_space			*f_mapping;
	void							*private_data;
	struct inode					*f_inode;
	unsigned int					f_flags;
	unsigned int					f_iocb_flags;
	char *						    f_path;
	union {
		struct mutex				f_pos_lock;
		u64							f_pipe;
	};
	loff_t							f_pos;
	void *							f_private;
	spinlock_t                      f_slock;
}__attribute__((aligned(sizeof(long)))) __randomize_layout;	


/********************************************************
 * 														*
 * 			   		  inode struct						*
 *														* 
*********************************************************/

#define IOP_FASTPERM	0x0001
#define IOP_LOOKUP	0x0002
#define IOP_NOFOLLOW	0x0004
#define IOP_XATTR	0x0008
#define IOP_DEFAULT_READLINK	0x0010
#define IOP_MGTIME	0x0020

/*
 * Keep mostly read-only and often accessed (especially for
 * the RCU path lookup and 'stat' data) fields at the beginning
 * of the 'struct inode'
 */

struct inode {
	umode_t            i_mode;        /* 文件模式（如：文件类型、权限） */
	unsigned int       i_flags;       /* 文件标志（如：不可变、同步） */

	const struct inode_operations *i_op;         /* inode 操作函数指针 */
	struct super_block            *i_sb;        /* 所属文件系统的超级块 */	
	struct address_space	      *i_mapping;   /* 指向文件数据所在的块设备的映射 */

	dev_t             i_rdev;         /* 设备号（如：字符设备、块设备） */

	u32			i_state;

	struct hlist_node	i_hash;
	struct list_head	i_io_list;	/* backing dev IO list */

	spinlock_t	        i_lock;	/* i_blocks, i_bytes, maybe i_size */
		

	struct list_head	i_sb_list;

	struct list_head	i_dentry;

	atomic_t		i_count;

	const struct file_operations	*i_fop;	/* former ->i_op->default_file_ops */
	void (*free_inode)(struct inode *);

	void			*i_private; /* fs or device private pointer */

}__attribute__((aligned(sizeof(long)))) __randomize_layout;

/********************************************************
 * 														*
 * 			   inode inode_operations					*
 *														* 
*********************************************************/

struct mnt_idmap {void * no_data;};

struct inode_operations {
	struct dentry  	 *(*lookup) 	   (struct inode *    ,struct dentry *, unsigned int);
	const char 	     *(*get_link) 	   (struct dentry *   , struct inode *, struct delayed_call *);
	int 			  (*permission)    (struct mnt_idmap *, struct inode *, int);
	struct posix_acl *(*get_inode_acl) (struct inode *	  , int, bool);
	int 			  (*readlink) 	   (struct dentry *   , char __user * ,int);
	int 			  (*create) 	   (struct mnt_idmap *, struct inode *,struct dentry *,umode_t, bool);
	int 			  (*link) 		   (struct dentry *   ,struct inode * ,struct dentry *);
	int 			  (*unlink) 	   (struct inode *	  ,struct dentry * );
	int 			  (*symlink) 	   (struct mnt_idmap *, struct inode *,struct dentry *,const char *);
	int 			  (*mkdir) 		   (struct mnt_idmap *, struct inode *,struct dentry *,umode_t);
	int 			  (*rmdir)		   (struct inode *	  ,struct dentry * );
	int 			  (*mknod) 		   (struct mnt_idmap *, struct inode *,struct dentry *,umode_t,dev_t);
	int 			  (*rename) 	   (struct mnt_idmap *, struct inode *, struct dentry *,struct inode *, struct dentry *, unsigned int);
	int 			  (*setattr)	   (struct mnt_idmap *, struct dentry*, struct iattr *);
	int 			  (*getattr) 	   (struct mnt_idmap *, const struct path *,struct kstat *, u32, unsigned int);
	ssize_t 		  (*listxattr) 	   (struct dentry *	  , char *		  , size_t);
	int 			  (*fiemap)		   (struct inode *	  , struct fiemap_extent_info *, u64 start,u64 len);
	int 			  (*update_time)   (struct inode *    , int);
	int 			  (*atomic_open)   (struct inode *    , struct dentry*,struct file *, unsigned open_flag,umode_t create_mode);
	int 			  (*tmpfile)	   (struct mnt_idmap *, struct inode *,struct file *, umode_t);
	struct posix_acl *(*get_acl)	   (struct mnt_idmap *, struct dentry*,int);
	int 			  (*set_acl)	   (struct mnt_idmap *, struct dentry*,struct posix_acl *, int);
	int 			  (*fileattr_set)  (struct mnt_idmap *idmap,struct dentry *dentry, struct fileattr *fa);
	int 			  (*fileattr_get)  (struct dentry *dentry  , struct fileattr *fa);
	struct offset_ctx*(*get_offset_ctx)(struct inode *inode);
};

/********************************************************
 * 														*
 * 			       file_operations						*
 *														* 
*********************************************************/

struct kiocb {
	struct file		*ki_filp;
	loff_t			ki_pos;
	void (*ki_complete)(struct kiocb *iocb, long ret);
	void			*private;
	int			ki_flags;
	u16			ki_ioprio; /* See linux/ioprio.h */
	union {
	//	struct wait_page_queue	*ki_waitq;
		ssize_t (*dio_complete)(void *data);
	};
};

struct dir_context;
typedef bool (*filldir_t)(struct dir_context *, const char *, int, loff_t, u64,
			 unsigned);

struct dir_context {
	filldir_t actor;
	loff_t pos;
};




typedef void (*poll_queue_proc)(struct file *, wait_queue_head_t *, struct poll_table_struct *);

typedef struct poll_table_struct {
	poll_queue_proc _qproc;
	__poll_t _key;
} poll_table;

typedef unsigned int __bitwise fop_flags_t;
typedef void *fl_owner_t;

struct file_operations {
	void *owner;
	fop_flags_t fop_flags;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iopoll)(struct kiocb *kiocb, struct io_comp_batch *,unsigned int flags);
	int (*iterate_shared) (struct file *, struct dir_context *);
	__poll_t (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
int (*mmap) (struct file *, struct vm_area_struct *);
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	void (*splice_eof)(struct file *file);
	int (*setlease)(struct file *, int, struct file_lease **, void **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,loff_t len);
	void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
	unsigned (*mmap_capabilities)(struct file *);
#endif
	ssize_t (*copy_file_range)(struct file *, loff_t, struct file *,loff_t, size_t, unsigned int);
	loff_t (*remap_file_range)(struct file *file_in, loff_t pos_in,struct file *file_out, loff_t pos_out,loff_t len, unsigned int remap_flags);
	int (*fadvise)(struct file *, loff_t, loff_t, int);
	int (*uring_cmd)(struct io_uring_cmd *ioucmd, unsigned int issue_flags);
	int (*uring_cmd_iopoll)(struct io_uring_cmd *, struct io_comp_batch *,unsigned int poll_flags);
};


/********************************************************
 * 														*
 * 			       super_block							*
 *														* 
*********************************************************/

struct rcu_sync {
	int			gp_state;
	int			gp_count;
	wait_queue_head_t	gp_wait;

	struct rcu_head		cb_head;
};

struct rcuwait {
	//struct task_struct __rcu *task;
};

struct percpu_rw_semaphore {
	struct rcu_sync		rss;
	unsigned int __percpu	*read_count;
	struct rcuwait		writer;
	wait_queue_head_t	waiters;
	atomic_t		block;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
};

/*
 * sb->s_flags.  Note that these mirror the equivalent MS_* flags where
 * represented in both.
 */
#define SB_RDONLY       BIT(0)	/* Mount read-only */
#define SB_NOSUID       BIT(1)	/* Ignore suid and sgid bits */
#define SB_NODEV        BIT(2)	/* Disallow access to device special files */
#define SB_NOEXEC       BIT(3)	/* Disallow program execution */
#define SB_SYNCHRONOUS  BIT(4)	/* Writes are synced at once */
#define SB_MANDLOCK     BIT(6)	/* Allow mandatory locks on an FS */
#define SB_DIRSYNC      BIT(7)	/* Directory modifications are synchronous */
#define SB_NOATIME      BIT(10)	/* Do not update access times. */
#define SB_NODIRATIME   BIT(11)	/* Do not update directory access times */
#define SB_SILENT       BIT(15)
#define SB_POSIXACL     BIT(16)	/* Supports POSIX ACLs */
#define SB_INLINECRYPT  BIT(17)	/* Use blk-crypto for encrypted files */
#define SB_KERNMOUNT    BIT(22)	/* this is a kern_mount call */
#define SB_I_VERSION    BIT(23)	/* Update inode I_version field */
#define SB_LAZYTIME     BIT(25)	/* Update the on-disk [acm]times lazily */

/* These sb flags are internal to the kernel */
#define SB_DEAD         BIT(21)
#define SB_DYING        BIT(24)
#define SB_SUBMOUNT     BIT(26)
#define SB_FORCE        BIT(27)
#define SB_NOSEC        BIT(28)
#define SB_BORN         BIT(29)
#define SB_ACTIVE       BIT(30)
#define SB_NOUSER       BIT(31)

/* These flags relate to encoding and casefolding */
#define SB_ENC_STRICT_MODE_FL	(1 << 0)

#define sb_has_strict_encoding(sb) \
	(sb->s_encoding_flags & SB_ENC_STRICT_MODE_FL)

/*
 *	Umount options
 */

#define MNT_FORCE	0x00000001	/* Attempt to forcibily umount */
#define MNT_DETACH	0x00000002	/* Just detach from the tree */
#define MNT_EXPIRE	0x00000004	/* Mark for expiry */
#define UMOUNT_NOFOLLOW	0x00000008	/* Don't follow symlink on umount */
#define UMOUNT_UNUSED	0x80000000	/* Flag guaranteed to be unused */

/* sb->s_iflags */
#define SB_I_CGROUPWB	0x00000001	/* cgroup-aware writeback enabled */
#define SB_I_NOEXEC	0x00000002	/* Ignore executables on this fs */
#define SB_I_NODEV	0x00000004	/* Ignore devices on this fs */
#define SB_I_STABLE_WRITES 0x00000008	/* don't modify blks until WB is done */

/* sb->s_iflags to limit user namespace mounts */
#define SB_I_USERNS_VISIBLE		0x00000010 /* fstype already mounted */
#define SB_I_IMA_UNVERIFIABLE_SIGNATURE	0x00000020
#define SB_I_UNTRUSTED_MOUNTER		0x00000040
#define SB_I_EVM_HMAC_UNSUPPORTED	0x00000080

#define SB_I_SKIP_SYNC	0x00000100	/* Skip superblock at global sync */
#define SB_I_PERSB_BDI	0x00000200	/* has a per-sb bdi */
#define SB_I_TS_EXPIRY_WARNED 0x00000400 /* warned about timestamp range expiry */
#define SB_I_RETIRED	0x00000800	/* superblock shouldn't be reused */
#define SB_I_NOUMASK	0x00001000	/* VFS does not apply umask */
#define SB_I_NOIDMAP	0x00002000	/* No idmapped mounts on this superblock */


enum {
	SB_UNFROZEN = 0,		/* FS is unfrozen */
	SB_FREEZE_WRITE	= 1,		/* Writes, dir ops, ioctls frozen */
	SB_FREEZE_PAGEFAULT = 2,	/* Page faults stopped as well */
	SB_FREEZE_FS = 3,		/* For internal FS use (e.g. to stop
					 	* internal threads if needed) */
	SB_FREEZE_COMPLETE = 4,		/* ->freeze_fs finished successfully */
};

#define SB_FREEZE_LEVELS (SB_FREEZE_COMPLETE - 1)

struct sb_writers {
	unsigned short			frozen;		/* Is sb frozen? */
	int				freeze_kcount;	/* How many kernel freeze requests? */
	int				freeze_ucount;	/* How many userspace freeze requests? */
	struct percpu_rw_semaphore	rw_sem[SB_FREEZE_LEVELS];
};

#define	UUID_STRING_LEN		36



struct super_block 
{
	struct list_head				s_list;		/* Keep this first */
    dev_t							s_dev;		/* search index; _not_ kdev_t */
    const struct super_operations	*s_op;
	const struct dquot_operations	*dq_op;
	struct dentry		*s_root;      //superblock的根目录结点
	struct block_device	*s_bdev; 
	struct hlist_node	s_instances;
	struct list_head	s_mounts;
	void			   *s_fs_info;	/* Filesystem private info */
	char			    s_sysfs_name[UUID_STRING_LEN + 1];
	unsigned int		s_max_links;
	struct mutex            		s_vfs_rename_mutex;	/* Kludge */
	const struct dentry_operations *s_d_op; /* default d_op for dentries */
	struct hlist_head s_pins;
};


/********************************************************
 * 														*
 * 			       super_operations						*
 *														* 
*********************************************************/

struct seq_operations ;
struct seq_file {
	char *buf;
	size_t size;
	size_t from;
	size_t count;
	size_t pad_until;
	loff_t index;
	loff_t read_pos;
	struct mutex lock;
	const struct seq_operations *op;
	int poll_event;
	const struct file *file;
	void *private;
};

struct seq_operations {
	void * (*start) (struct seq_file *m, loff_t *pos);
	void (*stop) (struct seq_file *m, void *v);
	void * (*next) (struct seq_file *m, void *v, loff_t *pos);
	int (*show) (struct seq_file *m, void *v);
};

enum freeze_holder {
	FREEZE_HOLDER_KERNEL	= (1U << 0),
	FREEZE_HOLDER_USERSPACE	= (1U << 1),
	FREEZE_MAY_NEST		= (1U << 2),
};

struct writeback_control {
	unsigned long nr_to_write;
};


struct shrink_control {
	gfp_t gfp_mask;
	int nid;
	unsigned long nr_to_scan;
	unsigned long nr_scanned;
    //struct mem_cgroup *memcg;
};

struct super_operations {
   	struct inode *(*alloc_inode)(struct super_block *sb);
	void (*destroy_inode)       (struct inode *);
	void (*free_inode)          (struct inode *);
   	void (*dirty_inode)         (struct inode *, int flags);
	int  (*write_inode)         (struct inode *, struct writeback_control *wbc);
	int  (*drop_inode)          (struct inode *);
	void (*evict_inode)         (struct inode *);
	void (*put_super)           (struct super_block *);
	int  (*sync_fs)             (struct super_block *sb, int wait);
	int  (*freeze_super)        (struct super_block *, enum freeze_holder who);
	int  (*freeze_fs)           (struct super_block *);
	int  (*thaw_super)          (struct super_block *, enum freeze_holder who);
	int  (*unfreeze_fs)         (struct super_block *);
	int  (*statfs)              (struct dentry *, struct kstatfs *);
	int  (*remount_fs)          (struct super_block *, int *, char *);
	void (*umount_begin)        (struct super_block *);
	int  (*show_options)        (struct seq_file *, struct dentry *);
	int  (*show_devname)        (struct seq_file *, struct dentry *);
	int  (*show_path)           (struct seq_file *, struct dentry *);
	int  (*show_stats)          (struct seq_file *, struct dentry *);

#ifdef CONFIG_QUOTA
	ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
	struct dquot __rcu **(*get_dquots)(struct inode *);
#endif

	long (*nr_cached_objects)(struct super_block *,struct shrink_control *);
	long (*free_cached_objects)(struct super_block *,struct shrink_control *);
	void (*shutdown)(struct super_block *sb);
};



/********************************************************
 * 														*
 * 			       dquot_operations						*
 *														* 
*********************************************************/

typedef __kernel_uid32_t 	qid_t; /* Type in which we store ids in memory */
typedef long long 			qsize_t;	/* Type in which we store sizes */

#undef USRQUOTA
#undef GRPQUOTA
#undef PRJQUOTA
enum quota_type {
	USRQUOTA = 0,		/* element used for user quotas */
	GRPQUOTA = 1,		/* element used for group quotas */
	PRJQUOTA = 2,		/* element used for project quotas */
};

struct kqid {			/* Type in which we store the quota identifier */
	union {
		kuid_t uid;
		kgid_t gid;
		kprojid_t projid;
	};
	enum quota_type type;  /* USRQUOTA (uid) or GRPQUOTA (gid) or PRJQUOTA (projid) */
};

struct mem_dqblk {
	qsize_t dqb_bhardlimit;	/* absolute limit on disk blks alloc */
	qsize_t dqb_bsoftlimit;	/* preferred limit on disk blks */
	qsize_t dqb_curspace;	/* current used space */
	qsize_t dqb_rsvspace;   /* current reserved space for delalloc*/
	qsize_t dqb_ihardlimit;	/* absolute limit on allocated inodes */
	qsize_t dqb_isoftlimit;	/* preferred inode limit */
	qsize_t dqb_curinodes;	/* current # allocated inodes */
	time64_t dqb_btime;	/* time limit for excessive disk use */
	time64_t dqb_itime;	/* time limit for excessive inode use */
};

struct dquot {
	struct hlist_node dq_hash;	/* Hash list in memory [dq_list_lock] */
	struct list_head dq_inuse;	/* List of all quotas [dq_list_lock] */
	struct list_head dq_free;	/* Free list element [dq_list_lock] */
	struct list_head dq_dirty;	/* List of dirty dquots [dq_list_lock] */
	struct mutex dq_lock;		/* dquot IO lock */
//	spinlock_t dq_dqb_lock;		/* Lock protecting dq_dqb changes */
	atomic_t dq_count;		/* Use count */
	struct super_block *dq_sb;	/* superblock this applies to */
	struct kqid dq_id;		/* ID this applies to (uid, gid, projid) */
	loff_t dq_off;			/* Offset of dquot on disk [dq_lock, stable once set] */
	unsigned long dq_flags;		/* See DQ_* */
	struct mem_dqblk dq_dqb;	/* Diskquota usage [dq_dqb_lock] */
};

/* Operations which must be implemented by each quota format */

struct quota_format_ops {
	int (*check_quota_file)(struct super_block *sb, int type);	/* Detect whether file is in our format */
	int (*read_file_info)(struct super_block *sb, int type);	/* Read main info about file - called on quotaon() */
	int (*write_file_info)(struct super_block *sb, int type);	/* Write main info about file */
	int (*free_file_info)(struct super_block *sb, int type);	/* Called on quotaoff() */
	int (*read_dqblk)(struct dquot *dquot);		/* Read structure for one user */
	int (*commit_dqblk)(struct dquot *dquot);	/* Write structure for one user */
	int (*release_dqblk)(struct dquot *dquot);	/* Called when last reference to dquot is being dropped */
	int (*get_next_id)(struct super_block *sb, struct kqid *qid);	/* Get next ID with existing structure in the quota file */
};

struct dquot_operations {
	int (*write_dquot) (struct dquot *);		/* Ordinary dquot write */
	struct dquot *(*alloc_dquot)(struct super_block *, int);	/* Allocate memory for new dquot */
	void (*destroy_dquot)(struct dquot *);		/* Free memory for dquot */
	int (*acquire_dquot) (struct dquot *);		/* Quota is going to be created on disk */
	int (*release_dquot) (struct dquot *);		/* Quota is going to be deleted from disk */
	int (*mark_dirty) (struct dquot *);		/* Dquot is marked dirty */
	int (*write_info) (struct super_block *, int);	/* Write of quota "superblock" */
	/* get reserved quota for delayed alloc, value returned is managed by
	 * quota code only */
	qsize_t *(*get_reserved_space) (struct inode *);
	int (*get_projid) (struct inode *, kprojid_t *);/* Get project ID */
	/* Get number of inodes that were charged for a given inode */
	int (*get_inode_usage) (struct inode *, qsize_t *);
	/* Get next ID with active quota structure */
	int (*get_next_id) (struct super_block *sb, struct kqid *qid);
};

/********************************************************
 * 														*
 * 			       file_system_type						*
 *														* 
*********************************************************/

struct fs_parameter_spec {
	const char		*name;
//	fs_param_type		*type;	/* The desired parameter type */
	u8			opt;	/* Option number (returned by fs_parse()) */
	unsigned short		flags;
#define fs_param_neg_with_no	0x0002	/* "noxxx" is negative param */
#define fs_param_can_be_empty	0x0004	/* "xxx=" is allowed */
#define fs_param_deprecated	0x0008	/* The param is deprecated */
	const void		*data;
};


struct file_system_type {
	const char *name;
	int fs_flags;
#define FS_REQUIRES_DEV		1 
#define FS_BINARY_MOUNTDATA	2
#define FS_HAS_SUBTYPE		4
#define FS_USERNS_MOUNT		8	/* Can be mounted by userns root */
#define FS_DISALLOW_NOTIFY_PERM	16	/* Disable fanotify permission events */
#define FS_ALLOW_IDMAP         32      /* FS has been updated to handle vfs idmappings. */
#define FS_MGTIME		64	/* FS uses multigrain timestamps */
#define FS_RENAME_DOES_D_MOVE	32768	/* FS will handle d_move() during rename() internally. */
	int (*init_fs_context)(struct fs_context *);
	const struct fs_parameter_spec *parameters;
	struct dentry *(*mount) (struct file_system_type *, int,
		       const char *, void *);
	void (*kill_sb) (struct super_block *);
	void *owner;
	struct file_system_type * next;
	struct hlist_head fs_supers;

	struct lock_class_key s_lock_key;
	struct lock_class_key s_umount_key;
	struct lock_class_key s_vfs_rename_key;
	struct lock_class_key s_writers_key[SB_FREEZE_LEVELS];
	struct lock_class_key i_lock_key;
	struct lock_class_key i_mutex_key;
	struct lock_class_key invalidate_lock_key;
	struct lock_class_key i_mutex_dir_key;
};



extern int register_filesystem(struct file_system_type *);
extern int unregister_filesystem(struct file_system_type *);
extern struct file_system_type *lookup_fs_type(const char *name);

/*--------------------------------------------------------------------------*/
static inline bool is_sync_kiocb(struct kiocb *kiocb)
{
	return kiocb->ki_complete == NULL;
}
static inline struct inode *file_inode(const struct file *f)
{
	return f->f_inode;
}
/*--------------------------------------------------------------------------*/




extern struct file *filp_open(const char * path, int flags, umode_t mode);
extern ssize_t kernel_read(struct file *file, void * buf, size_t count, loff_t *ppos);
extern ssize_t kernel_write(struct file *file,const void * buf, size_t count, loff_t *ppos);
extern long vfs_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
extern int file_close(struct file *file, fl_owner_t id);



/* fs/char_dev.c */
#define CHRDEV_MAJOR_MAX 512
/* Marks the bottom of the first segment of free char majors */
#define CHRDEV_MAJOR_DYN_END 234
/* Marks the top and bottom of the second segment of free char majors */
#define CHRDEV_MAJOR_DYN_EXT_START 511
#define CHRDEV_MAJOR_DYN_EXT_END 384



extern int alloc_chrdev_region(dev_t *, unsigned, unsigned, const char *);
extern int register_chrdev_region(dev_t, unsigned, const char *);
extern int __register_chrdev(unsigned int major, unsigned int baseminor,
    unsigned int count, const char *name,
    const struct file_operations *fops);
extern void __unregister_chrdev(unsigned int major, unsigned int baseminor,
		unsigned int count, const char *name);
extern void unregister_chrdev_region(dev_t, unsigned);

static inline int register_chrdev(unsigned int major, const char *name,
	const struct file_operations *fops)
{
return __register_chrdev(major, 0, 256, name, fops);
}

static inline void unregister_chrdev(unsigned int major, const char *name)
{
	__unregister_chrdev(major, 0, 256, name);
}





extern struct block_device *blkdev_get_by_path(char*, int,void*);




extern int __add_disk(struct gendisk *disk);
static int add_disk(struct gendisk *disk){
	return __add_disk(disk);
}



#endif /* _LINUX_FS_H */
