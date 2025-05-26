#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/mutex.h>


#define CHRDEV_MAJOR_HASH_SIZE 127

static DEFINE_MUTEX(chrdevs_lock);

static struct char_device_struct 
{
    struct char_device_struct *next; 
    unsigned int major; 
    unsigned int baseminor; 
    int minorct;
    char name[64]; 
    struct cdev *cdev; /* will die */ 
} *chrdevs[CHRDEV_MAJOR_HASH_SIZE];


static inline int major_to_index(unsigned major)
{
	return major % CHRDEV_MAJOR_HASH_SIZE;
}


static int find_dynamic_major(void)
{
	int i;
	struct char_device_struct *cd;
	for (i = ARRAY_SIZE(chrdevs)-1; i >= CHRDEV_MAJOR_DYN_END; i--) {
		if (chrdevs[i] == NULL)
			return i;
	}
	for (i = CHRDEV_MAJOR_DYN_EXT_START;
	     i >= CHRDEV_MAJOR_DYN_EXT_END; i--) {
		for (cd = chrdevs[major_to_index(i)]; cd; cd = cd->next)
			if (cd->major == i)
				break;
		if (cd == NULL)
			return i;
	}
	return -EBUSY;
}



static struct char_device_struct *
__register_chrdev_region(unsigned int major, unsigned int baseminor,
			   int minorct, const char *name)
{
	struct char_device_struct *cd, *curr, *prev = NULL;
	int ret;
	int i;

	if (major >= CHRDEV_MAJOR_MAX) {
		pr_err("CHRDEV \"%s\" major requested (%u) is greater than the maximum (%u)\n",
		       name, major, CHRDEV_MAJOR_MAX-1);
		return ERR_PTR(-EINVAL);
	}

	if (minorct > MINORMASK + 1 - baseminor) {
		pr_err("CHRDEV \"%s\" minor range requested (%u-%u) is out of range of maximum range (%u-%u) for a single major\n",
			name, baseminor, baseminor + minorct - 1, 0, MINORMASK);
		return ERR_PTR(-EINVAL);
	}

	cd = kzalloc(sizeof(struct char_device_struct), GFP_KERNEL);
	if (cd == NULL)
		return ERR_PTR(-ENOMEM);

	mutex_lock(&chrdevs_lock);

	if (major == 0) {
		ret = find_dynamic_major();
		if (ret < 0) {
			pr_err("CHRDEV \"%s\" dynamic allocation region is full\n",
			       name);
			goto out;
		}
		major = ret;
	}

	ret = -EBUSY;
	i = major_to_index(major);
	for (curr = chrdevs[i]; curr; prev = curr, curr = curr->next) {
		if (curr->major < major)
			continue;

		if (curr->major > major)
			break;

		if (curr->baseminor + curr->minorct <= baseminor)
			continue;

		if (curr->baseminor >= baseminor + minorct)
			break;

		goto out;
	}

	cd->major = major;
	cd->baseminor = baseminor;
	cd->minorct = minorct;
	strscpy(cd->name, name, sizeof(cd->name));

	if (!prev) {
		cd->next = curr;
		chrdevs[i] = cd;
	} else {
		cd->next = prev->next;
		prev->next = cd;
	}

	mutex_unlock(&chrdevs_lock);
	return cd;
out:
	mutex_unlock(&chrdevs_lock);
	kfree(cd);
	return ERR_PTR(ret);
}


static struct char_device_struct *
__unregister_chrdev_region(unsigned major, unsigned baseminor, int minorct)
{
	struct char_device_struct *cd = NULL, **cp;
	int i = major_to_index(major);

	mutex_lock(&chrdevs_lock);
	for (cp = &chrdevs[i]; *cp; cp = &(*cp)->next)
		if ((*cp)->major == major &&
		    (*cp)->baseminor == baseminor &&
		    (*cp)->minorct == minorct)
			break;
	if (*cp) {
		cd = *cp;
		*cp = cd->next;
	}
	mutex_unlock(&chrdevs_lock);
	return cd;
}


/**
 * register_chrdev_region() - register a range of device numbers
 * @from: the first in the desired range of device numbers; must include
 *        the major number.
 * @count: the number of consecutive device numbers required
 * @name: the name of the device or driver.
 *
 * Return value is zero on success, a negative error code on failure.
 */
int register_chrdev_region(dev_t from, unsigned count, const char *name)
{
	struct char_device_struct *cd;
	dev_t to = from + count;
	dev_t n, next;

	for (n = from; n < to; n = next) {
		next = MKDEV(MAJOR(n)+1, 0);
		if (next > to)
			next = to;
		cd = __register_chrdev_region(MAJOR(n), MINOR(n),
			       next - n, name);
		if (IS_ERR(cd))
			goto fail;
	}
	return 0;
fail:
	to = n;
	for (n = from; n < to; n = next) {
		next = MKDEV(MAJOR(n)+1, 0);
		kfree(__unregister_chrdev_region(MAJOR(n), MINOR(n), next - n));
	}
	return PTR_ERR(cd);
}

/**
 * alloc_chrdev_region() - register a range of char device numbers
 * @dev: output parameter for first assigned number
 * @baseminor: first of the requested range of minor numbers
 * @count: the number of minor numbers required
 * @name: the name of the associated device or driver
 *
 * Allocates a range of char device numbers.  The major number will be
 * chosen dynamically, and returned (along with the first minor number)
 * in @dev.  Returns zero or a negative error code.
 */
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
    const char *name)
{
struct char_device_struct *cd;
cd = __register_chrdev_region(0, baseminor, count, name);
if (IS_ERR(cd))
return PTR_ERR(cd);
*dev = MKDEV(cd->major, cd->baseminor);
return 0;
}

/**
 * __register_chrdev() - create and register a cdev occupying a range of minors
 * @major: major device number or 0 for dynamic allocation
 * @baseminor: first of the requested range of minor numbers
 * @count: the number of minor numbers required
 * @name: name of this range of devices
 * @fops: file operations associated with this devices
 *
 * If @major == 0 this functions will dynamically allocate a major and return
 * its number.
 *
 * If @major > 0 this function will attempt to reserve a device with the given
 * major number and will return zero on success.
 *
 * Returns a -ve errno on failure.
 *
 * The name of this device has nothing to do with the name of the device in
 * /dev. It only helps to keep track of the different owners of devices. If
 * your module name has only one type of devices it's ok to use e.g. the name
 * of the module here.
 */
int __register_chrdev(unsigned int major, unsigned int baseminor,
    unsigned int count, const char *name,
    const struct file_operations *fops)
{
    struct char_device_struct *cd;
    struct cdev *cdev;
    int err = -ENOMEM;

    cd = __register_chrdev_region(major, baseminor, count, name);
    if (IS_ERR(cd))
    return PTR_ERR(cd);

    cdev = cdev_alloc();
    if (!cdev)
    goto out2;

    cdev->owner = fops->owner;
    cdev->ops = fops;
    // kobject_set_name(&cdev->kobj, "%s", name);


    err = cdev_add(cdev, MKDEV(cd->major, baseminor), count);
    if (err)
    goto out;

    cd->cdev = cdev;

    return major ? 0 : cd->major;
out:
    //kobject_put(&cdev->kobj);
out2:
    kfree(__unregister_chrdev_region(cd->major, baseminor, count));
    return err;
}

/**
 * unregister_chrdev_region() - unregister a range of device numbers
 * @from: the first in the range of numbers to unregister
 * @count: the number of device numbers to unregister
 *
 * This function will unregister a range of @count device numbers,
 * starting with @from.  The caller should normally be the one who
 * allocated those numbers in the first place...
 */
void unregister_chrdev_region(dev_t from, unsigned count)
{
	dev_t to = from + count;
	dev_t n, next;

	for (n = from; n < to; n = next) {
		next = MKDEV(MAJOR(n)+1, 0);
		if (next > to)
			next = to;
		kfree(__unregister_chrdev_region(MAJOR(n), MINOR(n), next - n));
	}
}

/**
 * __unregister_chrdev - unregister and destroy a cdev
 * @major: major device number
 * @baseminor: first of the range of minor numbers
 * @count: the number of minor numbers this cdev is occupying
 * @name: name of this range of devices
 *
 * Unregister and destroy the cdev occupying the region described by
 * @major, @baseminor and @count.  This function undoes what
 * __register_chrdev() did.
 */
void __unregister_chrdev(unsigned int major, unsigned int baseminor,
    unsigned int count, const char *name)
{
struct char_device_struct *cd;

cd = __unregister_chrdev_region(major, baseminor, count);
if (cd && cd->cdev)
cdev_del(cd->cdev);
kfree(cd);
}




void cdev_put(struct cdev *p)
{
	if (p) {
		struct module *owner = p->owner;
		//kobject_put(&p->kobj);
		//module_put(owner);
	}
}


/**
 * cdev_add() - add a char device to the system
 * @p: the cdev structure for the device
 * @dev: the first device number for which this device is responsible
 * @count: the number of consecutive minor numbers corresponding to this
 *         device
 *
 * cdev_add() adds the device represented by @p to the system, making it
 * live immediately.  A negative error code is returned on failure.
 */
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
{
	int error;

	p->dev = dev;
	p->count = count;

	// if (WARN_ON(dev == WHITEOUT_DEV)) {
	// 	error = -EBUSY;
	// 	return error;
    //     goto err;
	// }

	// error = kobj_map(cdev_map, dev, count, NULL,
	// 		 exact_match, exact_lock, p);
	// if (error)
	// 	goto err;

	// kobject_get(p->kobj.parent);

	return 0;

// err:
// 	kfree_const(p->kobj.name);
// 	p->kobj.name = NULL;
// 	return error;
}





/**
 * cdev_alloc() - allocate a cdev structure
 *
 * Allocates and returns a cdev structure, or NULL on failure.
 */
struct cdev *cdev_alloc(void)
{
	struct cdev *p = kzalloc(sizeof(struct cdev), GFP_KERNEL);
	if (p) {
		INIT_LIST_HEAD(&p->list);
		// kobject_init(&p->kobj, &ktype_cdev_dynamic);
	}
	return p;
} 

/**
 * cdev_init() - initialize a cdev structure
 * @cdev: the structure to initialize
 * @fops: the file_operations for this device
 *
 * Initializes @cdev, remembering @fops, making it ready to add to the
 * system with cdev_add().
 */
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
	memset(cdev, 0, sizeof *cdev);
	INIT_LIST_HEAD(&cdev->list);
	// kobject_init(&cdev->kobj, &ktype_cdev_default);
	cdev->ops = fops;
}


/**
 * cdev_del() - remove a cdev from the system
 * @p: the cdev structure to be removed
 *
 * cdev_del() removes @p from the system, possibly freeing the structure
 * itself.
 *
 * NOTE: This guarantees that cdev device will no longer be able to be
 * opened, however any cdevs already open will remain and their fops will
 * still be callable even after cdev_del returns.
 */
void cdev_del(struct cdev *p)
{
    kfree(p);
    // cdev_unmap(p->dev, p->count);
	// kobject_put(&p->kobj);
}





struct file_operations *find_chrdev(unsigned int major, unsigned int minor)
{
	int i = major_to_index(major);
	struct char_device_struct *cd;

	mutex_lock(&chrdevs_lock);
	for (cd = chrdevs[i]; cd; cd = cd->next) {
		if (cd->major != major)
			continue;
		if (cd->baseminor <= minor && minor < cd->baseminor + cd->minorct) {
			mutex_unlock(&chrdevs_lock);
			return cd->cdev->ops;
		}
	}
	mutex_unlock(&chrdevs_lock);
	return NULL;
}
EXPORT_SYMBOL(find_chrdev);



/* Let modules do char dev stuff */
EXPORT_SYMBOL(register_chrdev_region);
EXPORT_SYMBOL(unregister_chrdev_region);
EXPORT_SYMBOL(alloc_chrdev_region);
EXPORT_SYMBOL(cdev_init);
EXPORT_SYMBOL(cdev_alloc);
EXPORT_SYMBOL(cdev_del);
EXPORT_SYMBOL(cdev_add);
// EXPORT_SYMBOL(cdev_set_parent);
// EXPORT_SYMBOL(cdev_device_add);
// EXPORT_SYMBOL(cdev_device_del);
EXPORT_SYMBOL(__register_chrdev);
EXPORT_SYMBOL(__unregister_chrdev);
