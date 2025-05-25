#ifndef __INITRAMFS_H_
#define __INITRAMFS_H_

#define READONLY 0
#define READABLE 1

typedef struct{
    char *path;
    int length;
    char *data;
    char mode;
}__file_data;

extern __file_data __export_file_start[];
extern __file_data __export_file_end[];



#define __used __attribute__((__used__))
#define __cold __attribute__((cold))

#define register_file(file_path,file_length,file_data,file_mode) \
    static __file_data __attribute__((__section__(".init_ramfs_files"))) \
    __used __cold register_file##_file_data = {     \
        .path   = file_path,    \
        .length = file_length,  \
        .data   = file_data,    \
        .mode   = file_mode     \
    }\
  
extern void initram_fs_mount_file(
    struct inode *inode,__file_data *filedata);


#endif // !1    