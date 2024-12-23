// SPDX-License-Identifier: GPL-2.0

/*
 * TODO: Brief description of this file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

// set shiftwidth=8 | set tabstop=8 | set softtabstop=8 | set noexpandtab

//#include "${BASENAME}.h"

#include <linux/version.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#ifndef __VER__
#define __VER__				"<none>"
#endif

#ifndef DRV_VER_MAJOR
#define DRV_VER_MAJOR			0
#endif

#ifndef DRV_VER_MINOR
#define DRV_VER_MINOR			1
#endif

#ifndef DRV_VER_RELEASE
#define DRV_VER_RELEASE			0
#endif

#ifndef DRIVER_VERSION
#define DRIVER_VERSION			__stringify(DRV_VER_MAJOR) "." __stringify(DRV_VER_MINOR) "." __stringify(DRV_VER_RELEASE)
#endif

#ifndef __DRVNAME__
#ifdef KBUILD_MODNAME
#define __DRVNAME__			KBUILD_MODNAME
#else
#define __DRVNAME__			"${BASENAME}"
#endif
#endif

#ifndef DEV_NAME
#define DEV_NAME			__DRVNAME__
#endif

#define DEV_COUNT			1

#define DEV_BUFSIZE			32

typedef struct ${BASENAME}
{
	struct file_operations file_ops;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	atomic_t open_count;
	char buf[DEV_BUFSIZE]; // For demo only, feel free to delete or modify it.
	// FIXME: Add more fields if needed, and delete this comment then.
} ${BASENAME}_t;

static ${BASENAME}_t s_${BASENAME};

static int ${BASENAME}_open(struct inode *inode, struct file *file)
{
	int open_count = atomic_inc_return(&s_${BASENAME}.open_count);

	if (open_count > 1)
	{
		dev_err(s_${BASENAME}.device, "Device has been opened %d times.\n", open_count);
		atomic_dec(&s_${BASENAME}.open_count);

		return -EMFILE;
	}

	if (file->f_flags & O_NONBLOCK)
		dev_notice(s_${BASENAME}.device, "Non-blocking mode enabled!\n");

	// FIXME: Add more operations if needed, and delete this comment then.

	file->private_data = &s_${BASENAME};

	return 0;
}

static int ${BASENAME}_release(struct inode *inode, struct file *file)
{
	${BASENAME}_t *${BASENAME} = (${BASENAME}_t *)file->private_data;

	if (${BASENAME})
	{
		// FIXME: Add more operations if needed, and delete this comment then.

		atomic_dec(&${BASENAME}->open_count);
		file->private_data = NULL;
	}

	return 0;
}

// FIXME: Contents below are just for demonstration, feel free to modify them and delete this comment line then.
static ssize_t ${BASENAME}_read(struct file *file, char __user *buf, size_t count, loff_t *off)
{
	ssize_t cnt = (count < sizeof(s_${BASENAME}.buf)) ? count : sizeof(s_${BASENAME}.buf);

	if (file->f_flags & O_NONBLOCK)
		; // No operation needed for this demo.

	return cnt - (ssize_t)copy_to_user(buf, s_${BASENAME}.buf, cnt);
}

// FIXME: Contents below are just for demonstration, feel free to modify them and delete this comment line then.
static ssize_t ${BASENAME}_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
	if (file->f_flags & O_NONBLOCK)
		; // No operation needed for this demo.

	if (count < sizeof(s_${BASENAME}.buf))
	{
		ssize_t cnt = ((ssize_t)count) - (ssize_t)copy_from_user(s_${BASENAME}.buf, buf, count);

		s_${BASENAME}.buf[cnt] = '\0';

		return cnt;
	}
#if 1
	return -EMSGSIZE;
#else
	return 0; // DON'T do this, otherwise the application program will hang if it uses fwrite().
#endif
}

static ${BASENAME}_t s_${BASENAME} = {
	.file_ops = {
		.owner = THIS_MODULE,
		.open = ${BASENAME}_open,
		.release = ${BASENAME}_release,
		.read = ${BASENAME}_read,
		.write = ${BASENAME}_write,
		// FIXME: Add more callbacks if needed, and delete this comment then.
	}
};

static __init int ${BASENAME}_init(void)
{
	dev_t dev_id;
	// FIXME: Add more variables if needed, and delete this comment then.
	int ret = 0;

	// FIXME: Add some pre-initializations if needed, and delete this comment then.

	if ((ret = alloc_chrdev_region(&dev_id, 0, DEV_COUNT, DEV_NAME)) < 0)
	{
		pr_err("[%s] alloc_chrdev_region() failed, ret = %d\n", DEV_NAME, ret);
		goto lbl_init_end;
	}

	s_${BASENAME}.cdev.owner = THIS_MODULE;
	cdev_init(&s_${BASENAME}.cdev, &s_${BASENAME}.file_ops);
	if ((ret = cdev_add(&s_${BASENAME}.cdev, dev_id, DEV_COUNT)) < 0)
	{
		pr_err("[%s] cdev_add() failed, ret = %d\n", DEV_NAME, ret);
		goto lbl_unreg_region;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
	s_${BASENAME}.class = class_create(THIS_MODULE, DEV_NAME);
#else
	s_${BASENAME}.class = class_create(DEV_NAME);
#endif
	if (IS_ERR(s_${BASENAME}.class))
	{
		ret = PTR_ERR(s_${BASENAME}.class);
		pr_err("[%s] class_create() failed, ret = %d\n", DEV_NAME, ret);
		goto lbl_del_cdev;
	}

	s_${BASENAME}.device = device_create(s_${BASENAME}.class, NULL, s_${BASENAME}.cdev.dev, NULL, DEV_NAME);
	if (IS_ERR(s_${BASENAME}.device))
	{
		ret = PTR_ERR(s_${BASENAME}.device);
		pr_err("[%s] device_create() failed, ret = %d\n", DEV_NAME, ret);
		goto lbl_destroy_class;
	}

	atomic_set(&s_${BASENAME}.open_count, 0);
	// FIXME: Add some post-initializations if needed, and delete this comment then.

	pr_notice("Initialized %s, driver version: %s-%s, kernel: %#x\n",
		DEV_NAME, DRIVER_VERSION, __VER__, LINUX_VERSION_CODE);

	goto lbl_init_end;

//lbl_destroy_dev:
//	device_destroy(s_${BASENAME}.class, s_${BASENAME}.cdev.dev);

lbl_destroy_class:
	class_destroy(s_${BASENAME}.class);

lbl_del_cdev:
	cdev_del(&s_${BASENAME}.cdev);

lbl_unreg_region:
	unregister_chrdev_region(s_${BASENAME}.cdev.dev, DEV_COUNT);

lbl_init_end:
	return ret;
}

static __exit void ${BASENAME}_exit(void)
{
	device_destroy(s_${BASENAME}.class, s_${BASENAME}.cdev.dev);
	class_destroy(s_${BASENAME}.class);
	cdev_del(&s_${BASENAME}.cdev);
	unregister_chrdev_region(s_${BASENAME}.cdev.dev, DEV_COUNT);

	pr_notice("Remove device %s.\n", DEV_NAME);
}

module_init(${BASENAME}_init);
module_exit(${BASENAME}_exit);

MODULE_DESCRIPTION("Driver for ${BASENAME}");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION "-" __VER__);
MODULE_AUTHOR("${LCS_USER} <${LCS_EMAIL}>");

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
