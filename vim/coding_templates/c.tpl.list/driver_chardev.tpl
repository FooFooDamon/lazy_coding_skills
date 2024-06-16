// SPDX-License-Identifier: GPL-2.0

/*
 * TODO: Brief description of this file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
*/

// set shiftwidth=8 | set tabstop=8 | set softtabstop=8 | set noexpandtab

//#include "${SELF_HEADER}.h"

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
#define __DRVNAME__			"${TITLE}"
#endif
#endif

#ifndef DEV_NAME
#define DEV_NAME			__DRVNAME__
#endif

#define DEV_COUNT			1

#define DEV_BUFSIZE			32

typedef struct ${TITLE}
{
	struct file_operations file_ops;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	atomic_t open_count;
	char buf[DEV_BUFSIZE]; // For demo only, feel free to delete or modify it.
	// FIXME: Add more fields if needed, and delete this comment then.
} ${TITLE}_t;

static ${TITLE}_t s_${TITLE};

static int ${TITLE}_open(struct inode *inode, struct file *file)
{
	int open_count = atomic_inc_return(&s_${TITLE}.open_count);

	if (open_count > 1)
	{
		dev_err(s_${TITLE}.device, "Device has been opened %d times.\n", open_count);
		atomic_dec(&s_${TITLE}.open_count);

		return -EMFILE;
	}

	if (file->f_flags & O_NONBLOCK)
		dev_notice(s_${TITLE}.device, "Non-blocking mode enabled!\n");

	// FIXME: Add more operations if needed, and delete this comment then.

	file->private_data = &s_${TITLE};

	return 0;
}

static int ${TITLE}_release(struct inode *inode, struct file *file)
{
	${TITLE}_t *${TITLE} = (${TITLE}_t *)file->private_data;

	if (${TITLE})
	{
		// FIXME: Add more operations if needed, and delete this comment then.

		atomic_dec(&${TITLE}->open_count);
		file->private_data = NULL;
	}

	return 0;
}

// FIXME: Contents below are just for demonstration, feel free to modify them and delete this comment line then.
static ssize_t ${TITLE}_read(struct file *file, char __user *buf, size_t count, loff_t *off)
{
	ssize_t cnt = (count < sizeof(s_${TITLE}.buf)) ? count : sizeof(s_${TITLE}.buf);

	if (file->f_flags & O_NONBLOCK)
		; // No operation needed for this demo.

	return cnt - (ssize_t)copy_to_user(buf, s_${TITLE}.buf, cnt);
}

// FIXME: Contents below are just for demonstration, feel free to modify them and delete this comment line then.
static ssize_t ${TITLE}_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
	if (file->f_flags & O_NONBLOCK)
		; // No operation needed for this demo.

	if (count < sizeof(s_${TITLE}.buf))
	{
		ssize_t cnt = ((ssize_t)count) - (ssize_t)copy_from_user(s_${TITLE}.buf, buf, count);

		s_${TITLE}.buf[cnt] = '\0';

		return cnt;
	}
#if 1
	return -EMSGSIZE;
#else
	return 0; // DON'T do this, otherwise the application program will hang if it uses fwrite().
#endif
}

static ${TITLE}_t s_${TITLE} = {
	.file_ops = {
		.owner = THIS_MODULE,
		.open = ${TITLE}_open,
		.release = ${TITLE}_release,
		.read = ${TITLE}_read,
		.write = ${TITLE}_write,
		// FIXME: Add more callbacks if needed, and delete this comment then.
	}
};

static __init int ${TITLE}_init(void)
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

	s_${TITLE}.cdev.owner = THIS_MODULE;
	cdev_init(&s_${TITLE}.cdev, &s_${TITLE}.file_ops);
	if ((ret = cdev_add(&s_${TITLE}.cdev, dev_id, DEV_COUNT)) < 0)
	{
		pr_err("[%s] cdev_add() failed, ret = %d\n", DEV_NAME, ret);
		goto lbl_unreg_region;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
	s_${TITLE}.class = class_create(THIS_MODULE, DEV_NAME);
#else
	s_${TITLE}.class = class_create(DEV_NAME);
#endif
	if (IS_ERR(s_${TITLE}.class))
	{
		ret = PTR_ERR(s_${TITLE}.class);
		pr_err("[%s] class_create() failed, ret = %d\n", DEV_NAME, ret);
		goto lbl_del_cdev;
	}

	s_${TITLE}.device = device_create(s_${TITLE}.class, NULL, s_${TITLE}.cdev.dev, NULL, DEV_NAME);
	if (IS_ERR(s_${TITLE}.device))
	{
		ret = PTR_ERR(s_${TITLE}.device);
		pr_err("[%s] device_create() failed, ret = %d\n", DEV_NAME, ret);
		goto lbl_destroy_class;
	}

	atomic_set(&s_${TITLE}.open_count, 0);
	// FIXME: Add some post-initializations if needed, and delete this comment then.

	pr_notice("Initialized %s, driver version: %s-%s, kernel: %#x\n",
		DEV_NAME, DRIVER_VERSION, __VER__, LINUX_VERSION_CODE);

	goto lbl_init_end;

//lbl_destroy_dev:
//	device_destroy(s_${TITLE}.class, s_${TITLE}.cdev.dev);

lbl_destroy_class:
	class_destroy(s_${TITLE}.class);

lbl_del_cdev:
	cdev_del(&s_${TITLE}.cdev);

lbl_unreg_region:
	unregister_chrdev_region(s_${TITLE}.cdev.dev, DEV_COUNT);

lbl_init_end:
	return ret;
}

static __exit void ${TITLE}_exit(void)
{
	device_destroy(s_${TITLE}.class, s_${TITLE}.cdev.dev);
	class_destroy(s_${TITLE}.class);
	cdev_del(&s_${TITLE}.cdev);
	unregister_chrdev_region(s_${TITLE}.cdev.dev, DEV_COUNT);

	pr_notice("Remove device %s.\n", DEV_NAME);
}

module_init(${TITLE}_init);
module_exit(${TITLE}_exit);

MODULE_DESCRIPTION("Driver for ${TITLE}");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION "-" __VER__);
MODULE_AUTHOR("${LCS_USER} <${LCS_EMAIL}>");

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial release.
 */
