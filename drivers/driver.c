#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/devfs.h>


struct  dentry * create_new_dentry(char * path,umode_t i_mode);

struct class *class_create(void *owner,char *name)
{
    pr_info("class_create: create class  %s\n",name);
    struct class *newclass = kmalloc(sizeof(struct class),GFP_KERNEL);
    if(newclass == NULL)
        return -ENOMEM;
    char* class_name = kmalloc(strlen(name)+ 1,GFP_KERNEL);
    if(class_name == NULL) 
        return -ENOMEM;
    strcpy(class_name,name);
    newclass->name = class_name;
    newclass->owner = owner;
    return newclass;
}


struct device *
device_create(const struct class *cls, struct device *parent, dev_t devt,
    void *drvdata, const char *fmt, ...){
    char devname[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(devname, sizeof(devname), fmt, args);
    va_end(args);

    char path[128];
    snprintf(path,128,"/dev/%s",devname);

    pr_info ("device_create: create char file %s \n",path);
    
    struct  dentry * dev_dentry = create_new_dentry(path , S_IFREG|0755);
    if(IS_ERR(dev_dentry)){
        return dev_dentry;
    }
    struct device *device = kmalloc(sizeof(struct device),GFP_KERNEL);
    if(device == NULL)
        return -ENOMEM; 
    device->parent = parent;
    device->class = cls;   
    struct file_operations * fop = find_chrdev(devt,0);

    devfs_mount_device(dev_dentry->d_inode,devt,fop); //此处会覆盖结点属性,之前的 S_IFREG|0755 实际上是无效的

    return device;
}


int __register_disk(struct block_device *dev,struct gendisk *disk,char *name,...)
{

    char devname[64];
    if(name!= NULL){
        va_list args;
        va_start(args, name);
        vsnprintf(devname, sizeof(devname), name, args);
        va_end(args);
    }
    else{
        strcpy(devname, disk->disk_name);
    }


    char path[128];
    snprintf(path,128,"/dev/%s",devname);

    pr_info ("device_create: create block file %s \n",path);

    struct  dentry * dev_dentry = create_new_dentry(path , S_IFREG|0755);
    if(IS_ERR(dev_dentry)){
        return dev_dentry;
    }

    devfs_mount_blk_device(dev_dentry->d_inode,dev,disk->major);

}
 







