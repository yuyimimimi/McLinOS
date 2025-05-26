 // SPDX-License-Identifier: GPL-2.0
/*
 * device.h - generic, centralized driver model
 *
 * Copyright (c) 2001-2003 Patrick Mochel <mochel@osdl.org>
 * Copyright (c) 2004-2009 Greg Kroah-Hartman <gregkh@suse.de>
 * Copyright (c) 2008-2009 Novell Inc.
 *
 * See Documentation/driver-api/driver-model/ for more information.
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <linux/list.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/uidgid.h>
#include <linux/overflow.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/devfs.h>

struct class {
    void * owner;
	const char		*name;
};



struct device {
// 	struct kobject kobj;
	struct device		*parent; 

// 	struct device_private	*p;

// 	const char		*init_name; /* initial name of the device */
// 	const struct device_type *type;

// 	const struct bus_type	*bus;	/* type of bus device is on */
// 	struct device_driver *driver;	/* which driver has allocated this
// 					   device */
// 	void		*platform_data;	/* Platform specific data, device
// 					   core doesn't touch it */
// 	void		*driver_data;	/* Driver data, set and get with
// 					   dev_set_drvdata/dev_get_drvdata */
// 	struct mutex		mutex;	/* mutex to synchronize calls to
// 					 * its driver.
// 					 */

// 	struct dev_links_info	links;
// 	struct dev_pm_info	power;
// 	struct dev_pm_domain	*pm_domain;

// #ifdef CONFIG_ENERGY_MODEL
// 	struct em_perf_domain	*em_pd;
// #endif

// #ifdef CONFIG_PINCTRL
// 	struct dev_pin_info	*pins;
// #endif
// 	struct dev_msi_info	msi;
// #ifdef CONFIG_ARCH_HAS_DMA_OPS
// 	const struct dma_map_ops *dma_ops;
// #endif
// 	u64		*dma_mask;	/* dma mask (if dma'able device) */
// 	u64		coherent_dma_mask;/* Like dma_mask, but for
// 					     alloc_coherent mappings as
// 					     not all hardware supports
// 					     64 bit addresses for consistent
// 					     allocations such descriptors. */
// 	u64		bus_dma_limit;	/* upstream dma constraint */
// 	const struct bus_dma_region *dma_range_map;

// 	struct device_dma_parameters *dma_parms;

// 	struct list_head	dma_pools;	/* dma pools (if dma'ble) */

// #ifdef CONFIG_DMA_DECLARE_COHERENT
// 	struct dma_coherent_mem	*dma_mem; /* internal for coherent mem
// 					     override */
// #endif
// #ifdef CONFIG_DMA_CMA
// 	struct cma *cma_area;		/* contiguous memory area for dma
// 					   allocations */
// #endif
// #ifdef CONFIG_SWIOTLB
// 	struct io_tlb_mem *dma_io_tlb_mem;
// #endif
// #ifdef CONFIG_SWIOTLB_DYNAMIC
// 	struct list_head dma_io_tlb_pools;
// 	spinlock_t dma_io_tlb_lock;
// 	bool dma_uses_io_tlb;
// #endif
// 	/* arch specific additions */
// 	struct dev_archdata	archdata;

// 	struct device_node	*of_node; /* associated device tree node */
// 	struct fwnode_handle	*fwnode; /* firmware device node */

// #ifdef CONFIG_NUMA
// 	int		numa_node;	/* NUMA node this device is close to */
// #endif
// 	dev_t			devt;	/* dev_t, creates the sysfs "dev" */
// 	u32			id;	/* device instance */

// 	spinlock_t		devres_lock;
// 	struct list_head	devres_head;

	const struct class	*class;
	// const struct attribute_group **groups;	/* optional groups */

	// void	(*release)(struct device *dev);
	// struct iommu_group	*iommu_group;
	// struct dev_iommu	*iommu;

	// struct device_physical_location *physical_location;

	// enum device_removable	removable;

	bool			offline_disabled:1;
	bool			offline:1;
	bool			of_node_reused:1;
	bool			state_synced:1;
	bool			can_match:1;
#if defined(CONFIG_ARCH_HAS_SYNC_DMA_FOR_DEVICE) || \
    defined(CONFIG_ARCH_HAS_SYNC_DMA_FOR_CPU) || \
    defined(CONFIG_ARCH_HAS_SYNC_DMA_FOR_CPU_ALL)
	bool			dma_coherent:1;
#endif
#ifdef CONFIG_DMA_OPS_BYPASS
	bool			dma_ops_bypass : 1;
#endif
#ifdef CONFIG_DMA_NEED_SYNC
	bool			dma_skip_sync:1;
#endif
#ifdef CONFIG_IOMMU_DMA
	bool			dma_iommu:1;
#endif
};


extern struct class *class_create(void *owner,char *name);

extern struct file_operations *find_chrdev(unsigned int major, unsigned int minor);




struct device *
device_create(const struct class *cls, struct device *parent, dev_t devt,
    void *drvdata, const char *fmt, ...);


#endif