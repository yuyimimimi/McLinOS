#include <linux/fs.h>
#include <linux/export.h>
#include <linux/uaccess.h>

static struct file* fd_table[256];  // 支持最多 256 个文件描述符

static int install_fd(struct file *file) {
    for (int i = 0; i < 256; ++i) {
        if (!fd_table[i]) {
            fd_table[i] = file;
            return i;
        }
    }
    return -EMFILE;  // Too many open files
}

static struct file* get_file_from_fd(int fd) {
    if (fd < 0 || fd >= 256) return NULL;
    return fd_table[fd];
}

static void release_fd(int fd) {
    if (fd >= 0 && fd < 256) {
        fd_table[fd] = NULL;
    }
}



static long sys_open(const char *filename, int flags, umode_t mode) {
    struct file *filp = filp_open(filename, flags, mode);
    if (IS_ERR(filp))
        return PTR_ERR(filp);

    int fd = install_fd(filp);
    if (fd < 0) {
        file_close(filp, NULL);
        return fd;
    }

    return fd;
}
EXPORT_SYMBOL(sys_open);


static long sys_read(unsigned int fd, char *buf, size_t count) {
    struct file *file = get_file_from_fd(fd);
    if (!file)
        return -EBADF;

    void *kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    loff_t pos = 0;
    ssize_t ret = kernel_read(file, kbuf, count, &pos);
    if (ret >= 0)
        if (copy_to_user(buf, kbuf, ret))
            ret = -EFAULT;

    kfree(kbuf);
    return ret;
}
EXPORT_SYMBOL(sys_read);



static long sys_write(unsigned int fd, const char *buf, size_t count) {
    struct file *file = get_file_from_fd(fd);
    if (!file)
        return -EBADF;

    void *kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    if (copy_from_user(kbuf, buf, count)) {
        kfree(kbuf);
        return -EFAULT;
    }

    loff_t pos = 0;
    ssize_t ret = kernel_write(file, kbuf, count, &pos);

    kfree(kbuf);
    return ret;
}
EXPORT_SYMBOL(sys_write);


static long sys_close(unsigned int fd) {
    struct file *file = get_file_from_fd(fd);
    if (!file)
        return -EBADF;

    file_close(file, NULL);
    release_fd(fd);
    return 0;
}
EXPORT_SYMBOL(sys_close);


struct dentry* mkdir(char *path,umode_t mode);
static int sys_mkdir(char* __user path,umode_t mode)
{
   return mkdir(path,mode);
}
EXPORT_SYMBOL(sys_mkdir);


static long sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg) {
    struct file *file = get_file_from_fd(fd);
    if (!file)
        return -EBADF;

    long ret = vfs_ioctl(file, cmd, arg);

    return ret;
}
EXPORT_SYMBOL(sys_ioctl);

