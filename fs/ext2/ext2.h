#ifndef  __EXT2_H_
#define  __EXT2_H_

#include <linux/types.h>


struct ext2_super_block {
    __u32   s_inodes_count;         /* inode 总数 */
    __u32   s_blocks_count;         /* 块总数 */
    __u32   s_r_blocks_count;       /* 为超级用户保留的块数 */
    __u32   s_free_blocks_count;    /* 空闲块总数 */
    __u32   s_free_inodes_count;    /* 空闲 inode 总数 */
    __u32   s_first_data_block;     /* 文件系统中第一个数据块 */
    __u32   s_log_block_size;       /* 用于计算块大小，表示值是2的幂的 * 1k。这个结果表示每一块的大小 */
    __u32   s_log_frag_size;        /* 用于计算片大小 */
    __u32   s_blocks_per_group;     /* 每组中块数 */
    __u32   s_frags_per_group;      /* 每组中片数 */
    __u32   s_inodes_per_group;     /* 每组中 inode 数。查找文件 inode 所在的块的位置很重要 */
    __u32   s_mtime;                /* 文件系统的挂载时间 */
    __u32   s_wtime;                /* 最后一次对该超级块进行写操作的时间 */
    __u16   s_mnt_count;            /* 文件系统挂载计数 */
    __u16   s_max_mnt_count;        /* 文件系统最大挂载计数 */
    __u16   s_magic;                /* 魔数签名，用于确定文件系统版本的标志 */
    __u16   s_state;                /* 文件系统的状态 */
    __u16   s_errors;               /* 检测错误时的行为 */
    __u16   s_minor_rev_level;      /* 次版本号 */
    __u32   s_lastcheck;            /* 上次检查的时间 */
    __u32   s_checkinterval;        /* 检查之间的最大间隔时间 */
    __u32   s_creator_os;           /* 创建的操作系統 */
    __u32   s_rev_level;            /* 版本号 */
    __u16   s_def_resuid;           /* 保留块的缺省 uid */
    __u16   s_def_resgid;           /* 保留块的缺省 gid */
    /* 下面这些区域是仅为 EXT2_DYNAM    IC_REV 超级块用的，在适应特征集和不适应特征集之间的差异是，
     * 如果在不适应特征集中设置了内核不认识的位，将拒绝挂接文件系统。
     * e2fsck 要求更严格，如果它不认识在适应特征集和不适应特征集中一个特征，它将退出而不去做它不知道的事情来干预。
     */
    __u32   s_first_ino;            /* 第一个非保留的索引节点号 */
    __u16   s_inode_size;           /* 索引节点结构的大小 */
    __u16   s_block_group_nr;       /* 该超级块所在的块组号 */
    __u32   s_feature_compat;       /* 相适应的特征集 */
    __u32   s_feature_incompat;     /* 不相适应的特征集 */
    __u32   s_feature_ro_compat;    /* 仅读适应的特征集 */
    __u8    s_uuid[16];             /* 卷的 128 位 uuid */
    char    s_volume_name[16];      /* 卷名 */
    char    s_last_mounted[64];     /* 上次挂载的目录 */
    __u32   s_algorithm_usage_bitmap; /* 压缩用的算法位图 */
    /* 执行时隐含，仅当 EXT2_COMPAT_PREALLOC 标志设置了时，目录预分配才发生。 */
    __u8    s_prealloc_blocks;      /* 试着预分配的块数 */
    __u8    s_prealloc_dir_blocks;  /* 目录预分配的数量 */
    __u16   s_padding1;
    __u32   s_reserved[204];        /* 保留结尾的填充 */
};

struct ext2_group_desc
{
	__u32 bg_block_bitmap;		/* 组中块位图所在的块号 */
	__u32 bg_inode_bitmap;		/* 组中索引节点位图所在的块号 */
	__u32 bg_inode_table;		/* 组中索引节点表的首块号 */
	__u16 bg_free_blocks_count;	/* 组中空闲块数 */
	__u16 bg_free_inodes_count;	/* 组中空闲索引节点数 */
	__u16 bg_used_dirs_count;	/* 组中已分配给目录的节点数 */
	__u16 bg_pad;
	__u32 bg_reserved[3];		/* 用 NULL 填充 12 个字节 */
};




#define Ext2_N_BLOCKS 15


#define S_IFMT  0170000  // 文件类型掩码
#define S_IFREG 0100000  // 普通文件
#define S_IFDIR 0040000  // 目录
#define S_IFLNK 0120000  // 符号链接
#define S_IFBLK 0060000  // 块设备
#define S_IFCHR 0020000  // 字符设备
#define S_IFIFO 0010000  // FIFO（管道）

struct ext2_inode {
	__u16 i_mode;		/* 文件类型和访问权限 */
	__u16 i_uid;		/* 文件拥有者标识号   */
	__u32 i_size;		/* 以字节计的文件大小 */
	__u32 i_atime;		/* 文件的最后一次访问时间 */
	__u32 i_ctime;		/* 该节点最后被修改时间   */
	__u32 i_mtime;		/* 文件内容的最后修改时间 */
	__u32 i_dtime;		/* 文件删除时间 */
	__u16 i_gid;		/* 文件的用户组标识符   */
	__u16 i_links_count;	/* 文件的硬链接计数 */
	__u32 i_blocks;		/* 文件所占块数 (每块以 512 字节计) */
	__u32 i_flags;		/* 打开文件的方式 */
    __u32 i_osd1; 
    __u32 i_block[Ext2_N_BLOCKS]; /* 指向数据块的指针数组 */
	__u32 i_version;	/* 文件的版本号 (用于 NFS) */
	__u32 i_file_acl;	/* 文件访问控制表 (已不再使用) */
	__u32 i_dir_acl;	/* 目录访问控制表 (已不再使用) */
	__u8  l_i_frag;		/* 每块中的片数 */
	__u32 l_i_faddr[3];	/* 片的地址     */	
};




struct ext2_inode_info  
{ 
    __u32 i_data[15];                /*数据块指针数组*/ 
    __u32 i_flags;                  /*打开文件的方式*/ 
    __u32 i_faddr;                  /*片的地址*/ 
    __u8  i_frag_no;                /*如果用到片，则是第一个片号*/ 
    __u8  i_frag_size;              /*片大小*/ 
    __u16 i_osync;                  /*同步*/ 
    __u32 i_file_acl;               /*文件访问控制链表*/ 
    __u32 i_dir_acl;                /*目录访问控制链表*/ 
    __u32 i_dtime;                  /*文件的删除时间*/ 
    __u32 i_block_group;            /*索引节点所在的块组号*/ 
    /******以下四个域是用于操作预分配块的*************/ 
    __u32 i_next_alloc_block;            
    __u32 i_next_alloc_goal; 
    __u32 i_prealloc_block; 
    __u32 i_prealloc_count; 
    __u32   i_dir_start_lookup;
    int     i_new_inode;   /* Is a freshly allocated inode */ 
}; 



struct ext2_dirent {
    __le32 inode;           
    __le16 rec_len;         
    unsigned char name_len; 
    unsigned char file_type;
    char name[1];           
};

#endif