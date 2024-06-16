// SPDX-License-Identifier: GPL-2.0

/*
 * Supplements to device class in linux mainline.
 *
 * Copyright (c) 2023-2024 Man Hung-Coeng <udc577@126.com>
*/

#include "devclass_supplements.h"

#include <linux/version.h>
#include <linux/device.h>

#ifdef __cplusplus
extern "C" {
#endif

int class_create_files(struct class *cls, const struct class_attribute *attrs)
{
    int err = 0;
    int i;

    for (i = 0; attrs[i].attr.name && !err; ++i)
    {
        err = class_create_file(cls, &attrs[i]);
    }

    if (err)
    {
        while (--i >= 0)
        {
            class_remove_file(cls, &attrs[i]);
        }
    }

    return err;
}

void class_remove_files(struct class *cls, const struct class_attribute *attrs)
{
    int i;

    for (i = 0; attrs[i].attr.name; ++i)
    {
        class_remove_file(cls, &attrs[i]);
    }
}

#ifdef TEST

#include <linux/fs.h>
#include <linux/cdev.h>

#ifndef __VER__
#define __VER__                         "<none>"
#endif

#define DEV_NAME                        "class_supp_demo"
#define DEV_MAX_NODE_COUNT              1
#define DEV_MINOR_NUM_START             0

typedef struct demo_device
{
    struct file_operations ops;
    struct cdev cdev;
    struct class *class;
    struct device *device;
} demo_device_t;

static demo_device_t s_dev = {
    .ops = {
        .owner = THIS_MODULE,
    },
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
#define DECLARE_SHOW_FUNC(_name)         \
    ssize_t _name##_show(struct class *cls, struct class_attribute *attr, char *buf)
#else
#define DECLARE_SHOW_FUNC(_name)         \
    ssize_t _name##_show(const struct class *cls, const struct class_attribute *attr, char *buf)
#endif

static DECLARE_SHOW_FUNC(version)
{
    return sprintf(buf, "%s\n", __VER__);
}

static DECLARE_SHOW_FUNC(max_node_count)
{
    return sprintf(buf, "%d\n", DEV_MAX_NODE_COUNT);
}

static DECLARE_SHOW_FUNC(minor_start)
{
    return sprintf(buf, "%d\n", DEV_MINOR_NUM_START);
}

static const struct class_attribute S_CLASS_ATTRS[] = {
    __ATTR_RO(version),
    __ATTR_RO(max_node_count),
    __ATTR_RO(minor_start),
    {} /* trailing empty sentinel*/
};

static __init int class_supp_demo_init(void)
{
    int i;
    dev_t devid;
    int ret = alloc_chrdev_region(&devid, DEV_MINOR_NUM_START, DEV_MAX_NODE_COUNT, DEV_NAME);

    if (ret < 0)
    {
        pr_err("[%s] alloc_chrdev_region() failed, ret = %d\n", DEV_NAME, ret);
        return ret;
    }

    s_dev.cdev.owner = THIS_MODULE;
    cdev_init(&s_dev.cdev, &s_dev.ops);

    if ((ret = cdev_add(&s_dev.cdev, devid, DEV_MAX_NODE_COUNT)) < 0)
    {
        pr_err("[%s] cdev_add() failed, ret = %d\n", DEV_NAME, ret);
        goto lbl_unreg_region;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
    s_dev.class = class_create(THIS_MODULE, DEV_NAME);
#else
    s_dev.class = class_create(DEV_NAME);
#endif
    if (IS_ERR(s_dev.class))
    {
        ret = PTR_ERR(s_dev.class);
        pr_err("[%s] class_create() failed, ret = %d\n", DEV_NAME, ret);
        goto lbl_del_cdev;
    }

    s_dev.device = device_create(s_dev.class, NULL, s_dev.cdev.dev, NULL, DEV_NAME);
    if (IS_ERR(s_dev.device))
    {
        ret = PTR_ERR(s_dev.device);
        pr_err("[%s] device_create() failed, ret = %d\n", DEV_NAME, ret);
        goto lbl_destroy_class;
    }

    if ((ret = class_create_files(s_dev.class, S_CLASS_ATTRS)) < 0)
    {
        pr_err("[%s] class_create_files() failed: %d\n", DEV_NAME, ret);
        goto lbl_destroy_device;
    }

    pr_notice("Initialized %s. Run following commands to check it:\n", DEV_NAME);
    for (i = 0; S_CLASS_ATTRS[i].attr.name; ++i)
    {
        pr_notice("[%s] cat /sys/class/%s/%s\n", DEV_NAME, DEV_NAME, S_CLASS_ATTRS[i].attr.name);
    }
    goto lbl_init_exit;

lbl_destroy_device:

    device_destroy(s_dev.class, s_dev.cdev.dev);

lbl_destroy_class:

    class_destroy(s_dev.class);

lbl_del_cdev:

    cdev_del(&s_dev.cdev);

lbl_unreg_region:

    unregister_chrdev_region(s_dev.cdev.dev, DEV_MAX_NODE_COUNT);

lbl_init_exit:

    return ret;
}

static __exit void class_supp_demo_exit(void)
{
    class_remove_files(s_dev.class, S_CLASS_ATTRS);
    device_destroy(s_dev.class, s_dev.cdev.dev);
    class_destroy(s_dev.class);
    cdev_del(&s_dev.cdev);
    unregister_chrdev_region(s_dev.cdev.dev, DEV_MAX_NODE_COUNT);
    pr_notice("Destroyed %s.\n", DEV_NAME);
}

module_init(class_supp_demo_init);
module_exit(class_supp_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Man Hung-Coeng <udc577@126.com>");

#endif /* #ifdef TEST */

#ifdef __cplusplus
}
#endif

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-12-17, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 *
 * >>> 2024-06-16, Man Hung-Coeng <udc577@126.com>:
 *  01. Fix the compilation error of class_create() on kernel 6.4.0 and above.
 */

