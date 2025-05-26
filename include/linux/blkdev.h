/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Portions Copyright (C) 1992 Drew Eckhardt
 */
#ifndef _LINUX_BLKDEV_H
#define _LINUX_BLKDEV_H

#include <linux/types.h>
#include <linux/blk-mq.h>
#include <linux/blk_types.h>
#include <linux/refcount_types.h>
#include <linux/lockdep_types.h>
#include <linux/blk_types.h>
#include <linux/workqueue_types.h>
#include <linux/blk.h>
#include <linux/pr.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>

struct gendisk;


/*
 * Maximum number of blkcg policies allowed to be registered concurrently.
 * Defined here to simplify include dependency.
 */
#define BLKCG_MAX_POLS		6

#define DISK_MAX_PARTS			256
#define DISK_NAME_LEN			32

#define PARTITION_META_INFO_VOLNAMELTH	64
/*
 * Enough for the string representation of any kind of UUID plus NULL.
 * EFI UUID is 36 characters. MSDOS UUID is 11 characters.
 */
#define PARTITION_META_INFO_UUIDLTH	(UUID_STRING_LEN + 1)

struct partition_meta_info {
	char uuid[PARTITION_META_INFO_UUIDLTH];
	u8 volname[PARTITION_META_INFO_VOLNAMELTH];
};

/**
 * DOC: genhd capability flags
 *
 * ``GENHD_FL_REMOVABLE``: indicates that the block device gives access to
 * removable media.  When set, the device remains present even when media is not
 * inserted.  Shall not be set for devices which are removed entirely when the
 * media is removed.
 *
 * ``GENHD_FL_HIDDEN``: the block device is hidden; it doesn't produce events,
 * doesn't appear in sysfs, and can't be opened from userspace or using
 * blkdev_get*. Used for the underlying components of multipath devices.
 *
 * ``GENHD_FL_NO_PART``: partition support is disabled.  The kernel will not
 * scan for partitions from add_disk, and users can't add partitions manually.
 *
 */
enum {
	GENHD_FL_REMOVABLE			= 1 << 0,
	GENHD_FL_HIDDEN				= 1 << 1,
	GENHD_FL_NO_PART			= 1 << 2,
};

enum {
	DISK_EVENT_MEDIA_CHANGE			= 1 << 0, /* media changed */
	DISK_EVENT_EJECT_REQUEST		= 1 << 1, /* eject requested */
};

enum {
	/* Poll even if events_poll_msecs is unset */
	DISK_EVENT_FLAG_POLL			= 1 << 0,
	/* Forward events to udev */
	DISK_EVENT_FLAG_UEVENT			= 1 << 1,
	/* Block event polling when open for exclusive write */
	DISK_EVENT_FLAG_BLOCK_ON_EXCL_WRITE	= 1 << 2,
};



enum blk_integrity_checksum {
	BLK_INTEGRITY_CSUM_NONE		= 0,
	BLK_INTEGRITY_CSUM_IP		= 1,
	BLK_INTEGRITY_CSUM_CRC		= 2,
	BLK_INTEGRITY_CSUM_CRC64	= 3,
} __packed ;

struct blk_integrity {
	unsigned char				flags;
	enum blk_integrity_checksum		csum_type;
	unsigned char				tuple_size;
	unsigned char				pi_offset;
	unsigned char				interval_exp;
	unsigned char				tag_size;
};


typedef unsigned int __bitwise blk_mode_t;

/* open for reading */
#define BLK_OPEN_READ		((__force blk_mode_t)(1 << 0))
/* open for writing */
#define BLK_OPEN_WRITE		((__force blk_mode_t)(1 << 1))
/* open exclusively (vs other exclusive openers */
#define BLK_OPEN_EXCL		((__force blk_mode_t)(1 << 2))
/* opened with O_NDELAY */
#define BLK_OPEN_NDELAY		((__force blk_mode_t)(1 << 3))
/* open for "writes" only for ioctls (specialy hack for floppy.c) */
#define BLK_OPEN_WRITE_IOCTL	((__force blk_mode_t)(1 << 4))
/* open is exclusive wrt all other BLK_OPEN_WRITE opens to the device */
#define BLK_OPEN_RESTRICT_WRITES	((__force blk_mode_t)(1 << 5))
/* return partition scanning errors */
#define BLK_OPEN_STRICT_SCAN	((__force blk_mode_t)(1 << 6))


enum block_device_flags_t
{
	BLOCK_DEVICE_FLAG_NOT_INITIALIZED,       //未初始化存储设备
    BLOCK_DEVICE_FLAG_MBR,                   //单mbr分区
    BLOCK_DEVICE_FLAG_GPT,                   //单gpt分区
    BLOCK_DEVICE_FLAG_PROTECTIVE_MBR ,       //保护MBR分区(gpt分区,为了兼容旧的操MBR分区)
    BLOCK_DEVICE_FLAG_BROKEN_MBR,            //损坏的MBR分区
    BLOCK_DEVICE_FLAG_BROKEN_GPT,            //损坏的GPT分区
    BLOCK_DEVICE_FLAG_BROKEN_PROTECTIVE_MBR, //损坏的保护MBR分区
};

static const char *block_device_flag_to_string(enum block_device_flags_t flag) {
    switch (flag) {
        case BLOCK_DEVICE_FLAG_MBR:
            return "BLOCK_DEVICE_FLAG_MBR";
        case BLOCK_DEVICE_FLAG_GPT:
            return "BLOCK_DEVICE_FLAG_GPT";
        case BLOCK_DEVICE_FLAG_PROTECTIVE_MBR:
            return "BLOCK_DEVICE_FLAG_PROTECTIVE_MBR";
        case BLOCK_DEVICE_FLAG_NOT_INITIALIZED:
            return "BLOCK_DEVICE_FLAG_NOT_INITIALIZED";
        case BLOCK_DEVICE_FLAG_BROKEN_MBR:
            return "BLOCK_DEVICE_FLAG_BROKEN_MBR";
        case BLOCK_DEVICE_FLAG_BROKEN_GPT:
            return "BLOCK_DEVICE_FLAG_BROKEN_GPT";
        case BLOCK_DEVICE_FLAG_BROKEN_PROTECTIVE_MBR:
            return "BLOCK_DEVICE_FLAG_BROKEN_PROTECTIVE_MBR";
        default:
            return "Unknown flag";
    }
}



struct gpt_header {
    uint64_t signature;             // 必须是 "EFI PART"
    uint32_t revision;              // 修订版本，通常为0x00010000
    uint32_t header_size;           // 头部大小（通常为92字节）
    uint32_t header_crc32;          // 头部的CRC32校验和
    uint32_t reserved;              // 保留字段
    uint64_t my_lba;                // 当前GPT表所在的LBA
    uint64_t backup_lba;            // 备份GPT表所在的LBA
    uint64_t first_usable_lba;      // 第一个可用的LBA（GPT分区的开始位置）
    uint64_t last_usable_lba;       // 最后一个可用的LBA（GPT分区的结束位置）
    uint8_t disk_guid[16];          // 磁盘的GUID
    uint64_t partition_entry_start_lba; // 分区表的起始LBA
    uint32_t partition_entry_count; // 分区表项的数量    
    uint32_t partition_entry_size;  // 每个分区表项的大小
    uint32_t partition_entry_crc32; // 分区表项的CRC32校验和
} __attribute__((packed));


struct gpt_partition_entry {
    uint8_t partition_type_guid[16];   // 分区类型的GUID
    uint8_t unique_partition_guid[16]; // 分区的唯一GUID
    uint64_t starting_lba;             // 分区的起始LBA
    uint64_t ending_lba;               // 分区的结束LBA
    uint64_t attributes;               // 分区属性
    uint16_t partition_name[36];       // 分区名称（UTF-16编码）
}__attribute__((packed));


struct gpt_partition {
    struct gpt_header header;
    struct gpt_partition_entry entries[8];
};


struct mbr_partition {
    uint8_t boot_ind;   // 启动标志（0x80表示活动分区）
    
    uint8_t start_head; 
    uint8_t start_sector; 
    uint8_t start_cyl;  

    uint8_t sys_ind;    // 系统id(分区类型)

    uint8_t end_head;   
    uint8_t end_sector; 
    uint8_t end_cyl;    
    
    uint32_t start_lba; // 分区的起始地址
    
    uint32_t nr_sectors; // 分区大小
}__attribute__((packed));


struct partition {
	struct gpt_partition gpt_partition;
	struct mbr_partition mbr_partition[4];   
};





struct gendisk {
	int major;
	int first_minor;
	int minors;
	char disk_name[DISK_NAME_LEN];	/* name of major driver */
	unsigned short events;		/* supported events */
	unsigned short event_flags;	/* flags related to event processing */
	struct xarray part_tbl;
	struct block_device *part0;
	const struct block_device_operations *fops; 
 	struct request_queue *queue;
	void *private_data;

//	struct bio_set bio_split;    /*为节约内存不使用*/

	int flags;
	unsigned long state;

#define GD_NEED_PART_SCAN		0
#define GD_READ_ONLY			1
#define GD_DEAD				2
#define GD_NATIVE_CAPACITY		3
#define GD_ADDED			4
#define GD_SUPPRESS_PART_SCAN		5
#define GD_OWNS_QUEUE			6

	struct mutex open_mutex;	/* open/close mutex */
	unsigned open_partitions;	/* number of open partitions */   
//	struct backing_dev_info	*bdi;
//	struct kobject queue_kobj;	/* the queue/ directory */
// 	struct kobject *slave_dir;
#ifdef CONFIG_BLOCK_HOLDER_DEPRECATED
	  struct list_head slave_bdevs;
#endif
	int node_id;
//	struct badblocks *bb;
	struct lockdep_map lockdep_map;
	u64 diskseq;
	blk_mode_t open_mode;
	struct blk_independent_access_ranges *ia_ranges;
};




typedef unsigned int __bitwise blk_features_t;
/* internal flags in queue_limits.flags */
typedef unsigned int __bitwise blk_flags_t;

struct queue_limits {
	blk_features_t		features;
	blk_flags_t		flags;
	unsigned long		seg_boundary_mask;
	unsigned long		virt_boundary_mask;

	unsigned int		max_hw_sectors;
	unsigned int		max_dev_sectors;
	unsigned int		chunk_sectors;
	unsigned int		max_sectors;
	unsigned int		max_user_sectors;
	unsigned int		max_segment_size;
	unsigned int		physical_block_size;
	unsigned int		logical_block_size;
	unsigned int		alignment_offset;
	unsigned int		io_min;
	unsigned int		io_opt;
	unsigned int		max_discard_sectors;
	unsigned int		max_hw_discard_sectors;
	unsigned int		max_user_discard_sectors;
	unsigned int		max_secure_erase_sectors;
	unsigned int		max_write_zeroes_sectors;
	unsigned int		max_hw_zone_append_sectors;
	unsigned int		max_zone_append_sectors;
	unsigned int		discard_granularity;
	unsigned int		discard_alignment;
	unsigned int		zone_write_granularity;

	/* atomic write limits */
	unsigned int		atomic_write_hw_max;
	unsigned int		atomic_write_max_sectors;
	unsigned int		atomic_write_hw_boundary;
	unsigned int		atomic_write_boundary_sectors;
	unsigned int		atomic_write_hw_unit_min;
	unsigned int		atomic_write_unit_min;
	unsigned int		atomic_write_hw_unit_max;
	unsigned int		atomic_write_unit_max;

	unsigned short		max_segments;
	unsigned short		max_integrity_segments;
	unsigned short		max_discard_segments;

	unsigned int		max_open_zones;
	unsigned int		max_active_zones;

	/*
	 * Drivers that set dma_alignment to less than 511 must be prepared to
	 * handle individual bvec's that are not a multiple of a SECTOR_SIZE
	 * due to possible offsets.
	 */
	unsigned int		dma_alignment;
	unsigned int		dma_pad_mask;

	struct blk_integrity	integrity;
};

struct blk_zone {
	uint8_t magic;
};

typedef int (*report_zones_cb)(struct blk_zone *zone, unsigned int idx,
    void *data);



// struct request_queue {

// 	void			*queuedata;           //存储私有数据
// 	struct elevator_queue	*elevator;   //指向io调度器的实现
// 	const struct blk_mq_ops	*mq_ops;     //指向块设备多队列
// 	struct blk_mq_ctx __percpu	*queue_ctx; //指向每个 CPU 的上下文结构
// 	unsigned long		queue_flags;         //队列的标志位
// 	unsigned int		rq_timeout;        //请求超时时间
// 	unsigned int		queue_depth;       //队列深度
// 	refcount_t		refs;              //引用计数
// 	unsigned int		nr_hw_queues;  //硬件队列数量
// 	struct xarray		hctx_table;     //硬件上下文（hardware context）的索引表。
// 	struct percpu_ref	q_usage_counter; //队列使用计数器。
// 	struct lock_class_key	io_lock_cls_key; //I/O 锁的调试信息。
// 	struct lockdep_map	io_lockdep_map;     
// 	struct lock_class_key	q_lock_cls_key;   //队列锁的调试信息。
// 	struct lockdep_map	q_lockdep_map;
// 	struct request		*last_merge;          //上一次合并的请求。
// 	spinlock_t		queue_lock;				  //队列的自旋锁。
// 	int			quiesce_depth;			     //静止（quiesce）深度
// 	struct gendisk		*disk;				//指向关联的磁盘设备。
// 	struct kobject *mq_kobj; //多队列的内核对象。用于 sysfs 文件系统，将队列的属性暴露给用户空间。
// 	struct queue_limits	limits;//队列的限制参数。定义了队列的物理和逻辑限制（如最大扇区数、最大段数等）。
// 	atomic_t		pm_only;     //电源管理相关的标志
// 	struct blk_queue_stats	*stats; //队列统计信息
// 	struct rq_qos		*rq_qos;         //请求服务质量（QoS）结构及其互斥锁。
// 	struct mutex		rq_qos_mutex;
// 	int			id;							//队列的唯一标识符
// 	unsigned long		nr_requests;	    //队列支持的最大请求数。
// 	struct timer_list	timeout;			//超时计时器和工作队列。
// 	struct work_struct	timeout_work;
// 	atomic_t		nr_active_requests_shared_tags; //共享标签的活跃请求数
// 	struct blk_mq_tags	*sched_shared_tags;           //调度器的共享标签。
// 	struct list_head	icq_list;          //I/O 上下文队列链表
// 	int			node;					//NUMA 节点 ID。
// 	spinlock_t		requeue_lock;    //重排队列的锁、链表和工作队列。
// 	struct list_head	requeue_list;
//     struct delayed_work	requeue_work;
// 	struct blk_flush_queue	*fq;        //刷新队列及其链表
// 	struct list_head	flush_list;
// 	struct mutex		sysfs_lock;          //sysfs 和限制参数的互斥锁。
// 	struct mutex		sysfs_dir_lock;
// 	struct mutex		limits_lock;
// 	struct list_head	unused_hctx_list;     //未使用的硬件上下文链表及其锁。
// 	spinlock_t		unused_hctx_lock;
// 	int			mq_freeze_depth;             //多队列冻结深度。
// 	struct rcu_head		rcu_head;           //RCU（Read-Copy-Update）释放钩子。
// 	wait_queue_head_t	mq_freeze_wq;         //多队列冻结的等待队列和锁。
// 	struct mutex		mq_freeze_lock; 
// 	struct blk_mq_tag_set	*tag_set;       //标签集及其链表。
// 	struct list_head	tag_set_list;
// 	struct dentry		*debugfs_dir;       //debugfs 文件系统的目录项。
// 	struct dentry		*sched_debugfs_dir;
// 	struct dentry		*rqos_debugfs_dir;
// 	struct mutex		debugfs_mutex;     //debugfs 操作的互斥锁。
// 	bool			mq_sysfs_init_done;    //多队列 sysfs 初始化完成标志。
// };


typedef void(*request_fn_t)(struct request *req);

struct request_queue {

	void			*queuedata;             //存储私有数据
	struct request		*last_merge;        //上一次合并的请求。
	spinlock_t		queue_lock;				//队列的自旋锁。
	int			quiesce_depth;			    //静止（quiesce）深度
	struct gendisk		*disk;				//指向关联的磁盘设备。
	struct queue_limits	limits;             //队列的限制参数。定义了队列的物理和逻辑限制（如最大扇区数、最大段数等）。
	int			id;							//队列的唯一标识符
	unsigned long		nr_requests;	    //队列支持的最大请求数。
	struct list_head	icq_list;           //I/O 上下文队列链表
	int			node;					    //NUMA 节点 ID。
	request_fn_t q_fn;                      //单队列设备操作
	spinlock_t*  q_lock;
};




static inline dev_t disk_devt(struct gendisk *disk)
{
	return MKDEV(disk->major, disk->first_minor);
}


/* blk_validate_limits() validates bsize, so drivers don't usually need to */
static inline int blk_validate_block_size(unsigned long bsize)
{
	if (bsize < 512 || bsize > PAGE_SIZE || !is_power_of_2(bsize))
		return -EINVAL;

	return 0;
}

// static inline bool blk_op_is_passthrough(blk_opf_t op)
// {
// 	op &= REQ_OP_MASK;
// 	return op == REQ_OP_DRV_IN || op == REQ_OP_DRV_OUT;
// }



extern struct request_queue *request_queue_init(int id, struct gendisk *gd,gfp_t flags);

extern void request_queue_add(struct request_queue *q, struct request *req);

extern void request_queue_remove(struct request_queue *q, struct request *req);

extern void process_requests_in_queue(struct request_queue *q);

extern struct request *blk_fetch_request(struct request_queue *q);

extern void __blk_insert_request(struct request *rq, struct bio *bio);

extern void __blk_cleanup_queue(struct request_queue *q);

static __always_inline struct request_queue *blk_alloc_queue(gfp_t flags){
	return request_queue_init(0, NULL, flags);
}
static __always_inline struct request_queue *blk_init_queue(request_fn_t fn,spinlock_t *lock){
	struct request_queue * q= request_queue_init(0, NULL, GFP_KERNEL);
	q->q_fn = fn;
	q->q_lock = lock;
	return q;
}
static __always_inline void blk_mq_insert_request(struct request_queue *q, struct request *req){
	request_queue_add(q, req);
}
static __always_inline void blk_queue_make_request(struct request_queue *q, struct request *req){
	request_queue_add(q, req);
}
static __always_inline void blk_mq_submit_bio(struct request *rq, struct bio *bio){  
	__blk_insert_request(rq, bio);
}
static __always_inline void blk_insert_request(struct request_queue *q, struct request *req){
	request_queue_add(q, req);
}
static __always_inline void blk_submit_bio(struct request *rq, struct bio *bio){  
	__blk_insert_request(rq, bio);
}

static __always_inline void blk_mq_end_request(struct request *rq,blk_status_t error){
	request_queue_remove(rq->q,rq);
}

static __always_inline void end_request(struct request *rq){
	request_queue_remove(rq->q,rq);
}
static void blk_cleanup_queue(struct request_queue *q){
	__blk_cleanup_queue(q);
}

extern void __put_disk(struct gendisk *disk);

static void put_disk(struct gendisk *disk){
	__put_disk(disk);
}

#define wait_for_completion(x) 

struct io_comp_batch {
	struct list_head req_list;
	bool need_ts;
	void (*complete)(struct io_comp_batch *);
};


enum blk_unique_id {
	/* these match the Designator Types specified in SPC */
	BLK_UID_T10	= 1,
	BLK_UID_EUI64	= 2,
	BLK_UID_NAA	= 3,
};


struct block_device_operations {
	void (*submit_bio)(struct bio *bio);
	int (*poll_bio)(struct bio *bio, struct io_comp_batch *iob,
			unsigned int flags);
	int (*open)(struct gendisk *disk, blk_mode_t mode);
	void (*release)(struct gendisk *disk);
	int (*ioctl)(struct block_device *bdev, blk_mode_t mode,
			unsigned cmd, unsigned long arg);
	int (*compat_ioctl)(struct block_device *bdev, blk_mode_t mode,
			unsigned cmd, unsigned long arg);
	unsigned int (*check_events) (struct gendisk *disk,
				      unsigned int clearing);
	void (*unlock_native_capacity) (struct gendisk *);
	int (*getgeo)(struct block_device *, struct hd_geometry *);
	int (*set_read_only)(struct block_device *bdev, bool ro);
	void (*free_disk)(struct gendisk *disk);
	/* this callback is with swap_lock and sometimes page table lock held */
	void (*swap_slot_free_notify) (struct block_device *, unsigned long);
	int (*report_zones)(struct gendisk *, sector_t sector,
			unsigned int nr_zones, report_zones_cb cb, void *data);
	char *(*devnode)(struct gendisk *disk, umode_t *mode);
	/* returns the length of the identifier or a negative errno: */
	int (*get_unique_id)(struct gendisk *disk, u8 id[16],
			enum blk_unique_id id_type);
	void *owner;
	const struct pr_ops *pr_ops;

	/*
	 * Special callback for probing GPT entry at a given sector.
	 * Needed by Android devices, used by GPT scanner and MMC blk
	 * driver.
	 */
	int (*alternative_gpt_sector)(struct gendisk *disk, sector_t *sector);
};


extern struct gendisk *__gendisk_alloc(int major,int minors);

static inline struct gendisk *alloc_disk(int minors){
	return __gendisk_alloc(0, minors);
}
 
extern void __put_disk(struct gendisk *disk);
static void del_gendisk(struct gendisk *disk){
	__put_disk(disk);
}







static __always_inline struct request * blk_get_request(struct request_queue *q , unsigned int op, gfp_t gfp_mask)
{
    if(!q) return NULL;
    struct request *rq =  request_alloc(q,gfp_mask, 1);
    rq->cmd_flags = op;
    return rq;
}



#define rq_for_each_bio(bio_, rq) \
    for(bio_ = rq->bio ; \
        rq->bio!= NULL;\
        rq->bio = rq->bio->bi_next ,bio_ = rq->bio)


#define bio_for_each_segment(bvec, bio, iter) \
    int bio_for_each_segment_number = 0;       \
    for( iter = &bio->bi_iter , bvec =  &bio->bi_io_vec[bio_for_each_segment_number]; \
         bio_for_each_segment_number < bio->bi_vcnt; \
         bio_for_each_segment_number++ ,bvec =  &bio->bi_io_vec[bio_for_each_segment_number])
 


#endif 
