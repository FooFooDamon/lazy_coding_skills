// SPDX-License-Identifier: GPL-2.0

/*
 * Chardev wrappers organized in groups.
 *
 * Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
*/

#include "chardev_group.h"

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>

#include "klogging.h"

#ifdef KLOGGING_USE_BASENAME
#define __FILE__                    "chardev_group.c"
#endif

static chardev_group_t *s_chardev_group = NULL;

chardev_group_t** __chardev_group_default_pptr(void)
{
    return &s_chardev_group;
}

chardev_group_t* chardev_group_create(const char *name, int baseminor, int max_items,
    const struct file_operations *fops)
{
    chardev_group_t *group = (chardev_group_t *)kzalloc(sizeof(*group), GFP_KERNEL);
    dev_t dev_id_start;
    int ret = (NULL == group) ? -ENOMEM : alloc_chrdev_region(&dev_id_start, baseminor, max_items, name);

    if (ret)
        goto lbl_free_group;

    if (NULL == (group->items = (struct device **)kzalloc(sizeof(struct device *) * max_items, GFP_KERNEL)))
    {
        pr_err_v("Failed to allocate memory for array of %d items!\n", max_items);
        ret = -ENOMEM;
        goto lbl_unreg_region;
    }

    group->cdev.owner = THIS_MODULE;
    cdev_init(&group->cdev, fops);
    if ((ret = cdev_add(&group->cdev, dev_id_start, max_items)) < 0)
    {
        pr_err_v("cdev_add() failed, ret = %d\n", ret);
        goto lbl_unreg_region;
    }

    group->class = class_create(THIS_MODULE, name);
    if (IS_ERR(group->class))
    {
        ret = PTR_ERR(group->class);
        pr_err_v("class_create() failed, ret = %d\n", ret);
        goto lbl_del_cdev;
    }

    return group;

lbl_del_cdev:

    cdev_del(&group->cdev);

lbl_unreg_region:

    unregister_chrdev_region(dev_id_start, max_items);

lbl_free_group:

    if (group)
    {
        if (group->items)
        {
            kfree(group->items);
            group->items = NULL;
        }

        kfree(group);
    }

    return ERR_PTR(ret);
}

void chardev_group_destroy(chardev_group_t **group)
{
    chardev_group_t *grp = group ? *group : NULL;

    if (!IS_ERR_OR_NULL(grp) && grp->class)
    {
        dev_t dev_id_start = grp->cdev.dev;
        int max_items = grp->cdev.count;

        class_destroy(grp->class);
        grp->class = NULL;
        cdev_del(&grp->cdev);
        unregister_chrdev_region(dev_id_start, max_items);
        kfree(grp->items);
        grp->items = NULL;
        kfree(grp);
    }

    *group = NULL;
}

static int __device_match_devt(struct device *dev, const void *pdevt)
{
    return dev->devt == *(dev_t *)pdevt;
}

static inline struct device* __class_find_device_by_devt(struct class *class, dev_t devt)
{
    return class_find_device(class, NULL, &devt, __device_match_devt);
}

struct device* chardev_group_make_item(chardev_group_t *group, const char *basename, void *private_data)
{
    if (!IS_ERR_OR_NULL(group) && group->class)
    {
        unsigned int major = MAJOR(group->cdev.dev);
        unsigned int i = MINOR(group->cdev.dev);
        unsigned int minor_end = i + group->cdev.count;

        for (; i < minor_end; ++i)
        {
            dev_t dev_id = MKDEV(major, i);
            struct device *dev_ptr = __class_find_device_by_devt(group->class, dev_id);

            if (NULL != dev_ptr)
                continue;

            if (NULL != (dev_ptr = device_create(group->class, NULL, dev_id, private_data, "%s%u", basename, i)))
            {
                group->items[group->cdev.count - (minor_end - i)] = dev_ptr;

                return dev_ptr;
            }
        }

        return ERR_PTR(-EDQUOT);
    }

    return ERR_PTR(-EINVAL);
}

void chardev_group_unmake_item(chardev_group_t *group, struct device *item)
{
    if (!IS_ERR_OR_NULL(group) && group->class && !IS_ERR_OR_NULL(item)
        && __class_find_device_by_devt(group->class, item->devt))
    {
        group->items[MINOR(item->devt) - MINOR(group->cdev.dev)] = NULL;
        device_destroy(group->class/* or item->class */, item->devt);
        pr_notice_v("Removed item[%s] from group[%s].\n", dev_name(item), group->class->name);
    }
}

static inline struct device* __chardev_group_find_item(dev_t dev_id, const chardev_group_t *group)
{
    if (!IS_ERR_OR_NULL(group) && group->items && MAJOR(dev_id) == MAJOR(group->cdev.dev))
    {
        unsigned int index = MINOR(dev_id) - MINOR(group->cdev.dev);

        return (index < group->cdev.count) ? group->items[index] : NULL;
    }

    return NULL;
}

struct device* chardev_group_find_item(dev_t dev_id, const chardev_group_t *group)
{
    return __chardev_group_find_item(dev_id, group);
}

void* chardev_group_find_item_private_data(dev_t dev_id, const chardev_group_t *group)
{
    struct device *dev = __chardev_group_find_item(dev_id, group);

    if (NULL == dev)
        pr_warn_v("Can not find device item with major = %d and minor = %d !\n", MAJOR(dev_id), MINOR(dev_id));

    return dev ? dev_get_drvdata(dev) : NULL;
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-11-03, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 */

