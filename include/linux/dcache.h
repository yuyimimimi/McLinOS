/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_DCACHE_H
#define __LINUX_DCACHE_H


#include <linux/list.h>
#include <linux/math.h>
#include <linux/cache.h>
#include <linux/wait.h>
#include <linux/lockref.h>


struct path;
struct file;
// struct vfsmount;

/*
 * linux/include/linux/dcache.h
 *
 * Dirent cache data structures
 *
 * (C) Copyright 1997 Thomas Schoebel-Theuer,
 * with heavy changes by Linus Torvalds
 */

#define IS_ROOT(x) ((x) == (x)->d_parent)

/* The hash is always the low bits of hash_len */
#ifdef __LITTLE_ENDIAN
 #define HASH_LEN_DECLARE u32 hash; u32 len
 #define bytemask_from_count(cnt)	(~(~0ul << (cnt)*8))
#else
 #define HASH_LEN_DECLARE u32 len; u32 hash
 #define bytemask_from_count(cnt)	(~(~0ul >> (cnt)*8))
#endif

/*
 * "quick string" -- eases parameter passing, but more importantly
 * saves "metadata" about the string (ie length and the hash).
 *
 * hash comes first so it snuggles against d_parent in the
 * dentry.
 */
struct qstr {
	union {
		struct {
			HASH_LEN_DECLARE;
		};
		u64 hash_len;
	};
	const unsigned char *name;
};

#define QSTR_INIT(n,l) { { { .len = l } }, .name = n }

extern const struct qstr empty_name;
extern const struct qstr slash_name;
extern const struct qstr dotdot_name;

/*
 * Try to keep struct dentry aligned on 64 byte cachelines (this will
 * give reasonable cacheline footprint with larger lines without the
 * large memory footprint increase).
 */
#ifdef CONFIG_64BIT
# define DNAME_INLINE_LEN 40 /* 192 bytes */
#else
# ifdef CONFIG_SMP
#  define DNAME_INLINE_LEN 36 /* 128 bytes */
# else
#  define DNAME_INLINE_LEN 44 /* 128 bytes */
# endif
#endif

 
struct dentry {

	struct hlist_head   d_hash;	/* lookup hash list */
	
	struct dentry      *d_parent;	/* parent directory */
	struct hlist_node   d_sib;	/* child of parent list */

	struct   qstr       d_name;
	unsigned char       d_iname[DNAME_INLINE_LEN];	/* small names */

	struct inode       *d_inode;		

	const struct dentry_operations *d_op;
	struct super_block *d_sb;	/* The root of the dentry tree */

	void               *d_fsdata;			/* fs-specific data */
		/* --- cacheline 2 boundary (128 bytes) --- */
	
	struct lockref d_lockref;	/* per-dentry lock and refcount
		* keep separate from RCU lookup area if
		* possible!
		*/

	spinlock_t          d_lock;
};


/*
 * dentry->d_lock spinlock nesting subclasses:
 *
 * 0: normal
 * 1: nested
 */
enum dentry_d_lock_class
{
	DENTRY_D_LOCK_NORMAL, /* implicitly used by plain spin_lock() APIs. */
	DENTRY_D_LOCK_NESTED
};

enum d_real_type {
	D_REAL_DATA,
	D_REAL_METADATA,
};

struct dentry_operations {
	int (*d_revalidate)(struct dentry *, unsigned int);
	int (*d_weak_revalidate)(struct dentry *, unsigned int);
	int (*d_hash)(const struct dentry *, struct qstr *);
	int (*d_compare)(const struct dentry *,
			unsigned int, const char *, const struct qstr *);
	int (*d_delete)(const struct dentry *);
	int (*d_init)(struct dentry *);
	void (*d_release)(struct dentry *);
	void (*d_prune)(struct dentry *);
	void (*d_iput)(struct dentry *, struct inode *);
	char *(*d_dname)(struct dentry *, char *, int);
	struct vfsmount *(*d_automount)(struct path *);
	int (*d_manage)(const struct path *, bool);
	struct dentry *(*d_real)(struct dentry *, enum d_real_type type);
} ____cacheline_aligned;

/*
 * Locking rules for dentry_operations callbacks are to be found in
 * Documentation/filesystems/locking.rst. Keep it updated!
 *
 * FUrther descriptions are found in Documentation/filesystems/vfs.rst.
 * Keep it updated too!
 */

/* d_flags entries */
#define DCACHE_OP_HASH			BIT(0)
#define DCACHE_OP_COMPARE		BIT(1)
#define DCACHE_OP_REVALIDATE		BIT(2)
#define DCACHE_OP_DELETE		BIT(3)
#define DCACHE_OP_PRUNE			BIT(4)

#define	DCACHE_DISCONNECTED		BIT(5)
     /* This dentry is possibly not currently connected to the dcache tree, in
      * which case its parent will either be itself, or will have this flag as
      * well.  nfsd will not use a dentry with this bit set, but will first
      * endeavour to clear the bit either by discovering that it is connected,
      * or by performing lookup operations.   Any filesystem which supports
      * nfsd_operations MUST have a lookup function which, if it finds a
      * directory inode with a DCACHE_DISCONNECTED dentry, will d_move that
      * dentry into place and return that dentry rather than the passed one,
      * typically using d_splice_alias. */

#define DCACHE_REFERENCED		BIT(6) /* Recently used, don't discard. */

#define DCACHE_DONTCACHE		BIT(7) /* Purge from memory on final dput() */

#define DCACHE_CANT_MOUNT		BIT(8)
#define DCACHE_GENOCIDE			BIT(9)
#define DCACHE_SHRINK_LIST		BIT(10)

#define DCACHE_OP_WEAK_REVALIDATE	BIT(11)

#define DCACHE_NFSFS_RENAMED		BIT(12)
     /* this dentry has been "silly renamed" and has to be deleted on the last
      * dput() */
#define DCACHE_FSNOTIFY_PARENT_WATCHED	BIT(14)
     /* Parent inode is watched by some fsnotify listener */

#define DCACHE_DENTRY_KILLED		BIT(15)

#define DCACHE_MOUNTED			BIT(16) /* is a mountpoint */
#define DCACHE_NEED_AUTOMOUNT		BIT(17) /* handle automount on this dir */
#define DCACHE_MANAGE_TRANSIT		BIT(18) /* manage transit from this dirent */
#define DCACHE_MANAGED_DENTRY \
	(DCACHE_MOUNTED|DCACHE_NEED_AUTOMOUNT|DCACHE_MANAGE_TRANSIT)

#define DCACHE_LRU_LIST			BIT(19)

#define DCACHE_ENTRY_TYPE		(7 << 20) /* bits 20..22 are for storing type: */
#define DCACHE_MISS_TYPE		(0 << 20) /* Negative dentry */
#define DCACHE_WHITEOUT_TYPE		(1 << 20) /* Whiteout dentry (stop pathwalk) */
#define DCACHE_DIRECTORY_TYPE		(2 << 20) /* Normal directory */
#define DCACHE_AUTODIR_TYPE		(3 << 20) /* Lookupless directory (presumed automount) */
#define DCACHE_REGULAR_TYPE		(4 << 20) /* Regular file type */
#define DCACHE_SPECIAL_TYPE		(5 << 20) /* Other file type */
#define DCACHE_SYMLINK_TYPE		(6 << 20) /* Symlink */

#define DCACHE_NOKEY_NAME		BIT(25) /* Encrypted name encoded without key */
#define DCACHE_OP_REAL			BIT(26)

#define DCACHE_PAR_LOOKUP		BIT(28) /* being looked up (with parent locked shared) */
#define DCACHE_DENTRY_CURSOR		BIT(29)
#define DCACHE_NORCU			BIT(30) /* No RCU delay for freeing */





extern int dentry_rename(struct dentry *d,char *name);
extern struct dentry *__d_alloc(struct super_block *sb, const char *name);
extern struct dentry *d_alloc(struct dentry * parent, const char *name);
extern char * d_getname(struct dentry * dentry);
extern void d_delete(struct dentry * dentry); //释放dentry。并减少inode的引用计数。
extern struct dentry *d_lookup_dentry(struct dentry *parent, const char *name); //查询缓存中的结点
extern void __d_drop(struct dentry *dentry); //撤销dentry创建，只适用于分配后未使用的dentry

extern struct inode* new_inode(struct super_block *sb);
extern void destroy_inode(struct inode *node);
extern void inode_get(struct inode *inode);
extern void inode_put(struct inode *inode);

extern struct super_block *alloc_super(struct block_device *);
extern void put_super(struct super_block *);

extern struct dentry * d_add(struct dentry *dentry, struct inode *inode);
extern int simple_unlink(struct inode *dir, struct dentry *dentry); //将denrty从父目录的缓存中清除后调用d_delete

extern struct file * f_get(struct dentry * dentry);
extern struct file* f_put(struct file * file);



// extern struct inode* new_inode(struct super_block *sb);
// extern void   inode_put	 	  (struct inode *inode);

 //暂时未空函数，所以目前还不支持目录缓存
// extern int simple_unlink(struct inode *dir ,struct dentry * dentry);

// extern void d_put(struct dentry *dentry);


// extern struct dentry * d_alloc     (struct dentry * parent, const struct qstr *name);   //创建一个dentry
// extern struct dentry * d_alloc_anon(struct super_block *);
// extern void   d_delete        (struct dentry * dentry);                                      //删除一个dentry
// extern void   destroy_inode   (struct inode *node);                                         
// extern void   inode_get       (struct inode *inode);

 



/* Allocation counts.. */

/**
 * dget_dlock -	get a reference to a dentry
 * @dentry: dentry to get a reference to
 *
 * Given a live dentry, increment the reference count and return the dentry.
 * Caller must hold @dentry->d_lock.  Making sure that dentry is alive is
 * caller's resonsibility.  There are many conditions sufficient to guarantee
 * that; e.g. anything with non-negative refcount is alive, so's anything
 * hashed, anything positive, anyone's parent, etc.
 */
static inline struct dentry *dget_dlock(struct dentry *dentry)
{
	dentry->d_lockref.count++;
	return dentry;
}
static inline struct dentry *dput_dlock(struct dentry *dentry)
{
	dentry->d_lockref.count--;
	return dentry;
}

#endif /* __LINUX_DCACHE_H */