// SPDX-License-Identifier: GPL-2.0

/*
 * Chardev wrappers organized in groups.
 *
 * Copyright (c) 2023-2024 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
*/

#include "chardev_group.h"

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include "klogging.h"

#ifdef KLOGGING_USE_BASENAME
#define __FILE__                    "chardev_group.c"
#endif

typedef struct chardev_group
{
    struct device **items;
    struct class *class;
    struct cdev cdev;
} chardev_group_t;

static chardev_group_t *s_chardev_group = NULL;

struct chardev_group** __chardev_group_default_pptr(void)
{
    return &s_chardev_group;
}

static const char *S_PROPERTIES[] = {
    "items:struct device **",
    "class:struct class *",
    "cdev:struct cdev *",
    NULL /* sentinel */
};

const char** chardev_group_available_properties(void)
{
    return S_PROPERTIES;
}

void* chardev_group_get_property(const char *prop_name, struct chardev_group* group)
{
    if (unlikely(NULL == group || NULL == prop_name))
        return ERR_PTR(-EFAULT);

    if (0 == strcmp(prop_name, "items"))
        return group->items;

    if (0 == strcmp(prop_name, "class"))
        return group->class;

    if (0 == strcmp(prop_name, "cdev"))
        return &group->cdev;

    return ERR_PTR(-EINVAL);
}

struct chardev_group* chardev_group_create(const char *name, unsigned int baseminor, unsigned int max_items,
    const struct file_operations *fops)
{
    chardev_group_t *group = (chardev_group_t *)kzalloc(sizeof(chardev_group_t), GFP_KERNEL);
    dev_t dev_id_start;
    int ret = (NULL == group) ? -ENOMEM : alloc_chrdev_region(&dev_id_start, baseminor, max_items, name);

    if (ret)
        goto lbl_free_group;

    if (NULL == (group->items = (struct device **)kzalloc(sizeof(struct device *) * max_items, GFP_KERNEL)))
    {
        pr_err_v("Failed to allocate memory for array of %d items of group[%s]!\n", max_items, name);
        ret = -ENOMEM;
        goto lbl_unreg_region;
    }

    group->cdev.owner = THIS_MODULE;
    cdev_init(&group->cdev, fops);
    if ((ret = cdev_add(&group->cdev, dev_id_start, max_items)) < 0)
    {
        pr_err_v("cdev_add() for group[%s] failed, err = %d\n", name, ret);
        goto lbl_unreg_region;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
    group->class = class_create(THIS_MODULE, name);
#else
    group->class = class_create(name);
#endif
    if (IS_ERR(group->class))
    {
        ret = PTR_ERR(group->class);
        pr_err_v("class_create(%s) failed, err = %d\n", name, ret);
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

void chardev_group_destroy(struct chardev_group **group, void (*free_privdata)(const void *))
{
    chardev_group_t *grp = group ? *group : NULL;

    if (!IS_ERR_OR_NULL(grp) && grp->class)
    {
        dev_t dev_id_start = grp->cdev.dev;
        unsigned int max_items = grp->cdev.count;

        if (free_privdata)
        {
            unsigned int i;

            for (i = 0; i < max_items; ++i)
            {
                chardev_group_unmake_item(grp, grp->items[i], free_privdata);
            }
        }
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

struct device* chardev_group_make_item(struct chardev_group *group, const char *basename, void *private_data)
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

            if (IS_ERR(dev_ptr = device_create(group->class, NULL, dev_id, private_data, "%s%u", basename, i)))
                pr_err_v("device_create(%s%u) failed, err = %ld\n", basename, i, PTR_ERR(dev_ptr));
            else
            {
                group->items[group->cdev.count - (minor_end - i)] = dev_ptr;
                pr_notice_v("Created item[%s] and added it to group[%s].\n", dev_name(dev_ptr), group->class->name);
            }

            return dev_ptr;
        }

        return ERR_PTR(-EDQUOT);
    }

    return ERR_PTR(-EINVAL);
}

void chardev_group_unmake_item(struct chardev_group *group, struct device *item, void (*free_privdata)(const void *))
{
    if (!IS_ERR_OR_NULL(group) && group->class && !IS_ERR_OR_NULL(item)
        && __class_find_device_by_devt(group->class, item->devt))
    {
        group->items[MINOR(item->devt) - MINOR(group->cdev.dev)] = NULL;
        if (free_privdata)
        {
            free_privdata(dev_get_drvdata(item));
            dev_set_drvdata(item, NULL);
        }
        device_destroy(group->class/* or item->class */, item->devt);
        pr_notice_v("Removed item[%s] from group[%s].\n", dev_name(item), group->class->name);
    }
}

static inline struct device* __chardev_group_find_item(dev_t dev_id, const struct chardev_group *group)
{
    if (!IS_ERR_OR_NULL(group) && group->items && MAJOR(dev_id) == MAJOR(group->cdev.dev))
    {
        unsigned int index = MINOR(dev_id) - MINOR(group->cdev.dev);

        return (index < group->cdev.count) ? group->items[index] : NULL;
    }

    return NULL;
}

struct device* chardev_group_find_item(dev_t dev_id, const struct chardev_group *group)
{
    return __chardev_group_find_item(dev_id, group);
}

void* chardev_group_find_item_private_data(dev_t dev_id, const struct chardev_group *group)
{
    struct device *dev = __chardev_group_find_item(dev_id, group);

    if (NULL == dev)
        pr_warn_v("Can not find device item with major = %d and minor = %d !\n", MAJOR(dev_id), MINOR(dev_id));

    return dev ? dev_get_drvdata(dev) : NULL;
}

#ifdef TEST

#include <linux/module.h>

#define DEFAULT_MINOR_BASE                  9527
#define DEFAULT_NODE_COUNT                  3
#define DEFAULT_GRP_VAR_FLAG                0

static uint mibase = DEFAULT_MINOR_BASE;
module_param(mibase, uint, 0644);
MODULE_PARM_DESC(mibase, " starting/base value of device minor number (default: " __stringify(DEFAULT_MINOR_BASE) ")");

static uint ncount = DEFAULT_NODE_COUNT;
module_param(ncount, uint, 0644);
MODULE_PARM_DESC(ncount, " how many device nodes (default: " __stringify(DEFAULT_NODE_COUNT) ")");

static bool gvar = DEFAULT_GRP_VAR_FLAG;
module_param(gvar, bool, 0644);
MODULE_PARM_DESC(gvar, " group variable: 0 to use the pre-defined variable, 1 to explicitly define a new one. (default: "
    __stringify(DEFAULT_GRP_VAR_FLAG) ")");

typedef struct demo_struct
{
    struct device *device;
    atomic_t open_count;
    /* More fields depending on your need. */
} demo_struct_t;

static chardev_group_t *s_user_group = NULL;

static int chardev_group_demo_open(struct inode *inode, struct file *file)
{
    demo_struct_t *demo_item = (demo_struct_t *)(
        (DEFAULT_GRP_VAR_FLAG == gvar)
        ? CHRDEV_GRP_FIND_ITEM_PRIVDATA_BY_INODE(inode)
        : chardev_group_find_item_private_data(inode->i_rdev, s_user_group)
    );
    int open_count = demo_item ? atomic_inc_return(&demo_item->open_count) : 2;

    if (demo_item)
    {
        const char *devname = dev_name(demo_item->device);

        if (open_count > 1)
        {
            pr_err_v("Device[%s] has been opened %d times.\n", devname, open_count);
            atomic_dec(&demo_item->open_count);
            return -EMFILE;
        }

        file->private_data = demo_item;
        pr_info_v("Opened device[%s]: priv_addr = 0x%p\n", devname, file->private_data);
    }

    return 0;
}

static int chardev_group_demo_release(struct inode *inode, struct file *file)
{
    demo_struct_t *demo_item = (demo_struct_t *)(
        (DEFAULT_GRP_VAR_FLAG == gvar)
        ? CHRDEV_GRP_FIND_ITEM_PRIVDATA_BY_INODE(inode)
        : chardev_group_find_item_private_data(inode->i_rdev, s_user_group)
    );

    if (demo_item)
    {
        pr_info_v("Closed device[%s]: priv_addr = 0x%p\n", dev_name(demo_item->device), file->private_data);
        atomic_dec(&demo_item->open_count);
        file->private_data = NULL;
    }

    return 0;
}

static ssize_t chardev_group_demo_read(struct file *file, char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

static ssize_t chardev_group_demo_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return -EOPNOTSUPP;
}

static const struct file_operations S_FILE_OPS = {
    .owner = THIS_MODULE,
    .open = chardev_group_demo_open,
    .release = chardev_group_demo_release,
    .read = chardev_group_demo_read,
    .write = chardev_group_demo_write,
};

#define DEFAULT_GRP_ITEM_BASENAME               "default_chrdev_grp_"
#define USER_GRP_ITEM_BASENAME                  "user_chrdev_grp_"

static int __default_group_demo_init(void)
{
    uint i;
    int ret = 0;

    if (IS_ERR(CHRDEV_GRP_CREATE(KBUILD_MODNAME, mibase, ncount, &S_FILE_OPS)))
    {
        ret = PTR_ERR(THIS_CHRDEV_GRP);
        goto lbl_init_end;
    }

    for (i = 0; i < ncount; ++i) /* NOTE: There's no loop in xxx_probe(). */
    {
        demo_struct_t *demo_item = (demo_struct_t *)kzalloc(sizeof(demo_struct_t), GFP_KERNEL);

        if (NULL == demo_item)
        {
            /* ret = -ENOMEM; */ /* Don't modify ret if partial failure is allowed. */
            pr_err_v("Failed to allocate memory for demo item [%u]\n", i);
            break;
        }

        if (IS_ERR(demo_item->device = CHRDEV_GRP_MAKE_ITEM(DEFAULT_GRP_ITEM_BASENAME, demo_item)))
        {
            /* ret = PTR_ERR(demo_item->device); */ /* Don't modify ret if partial failure is allowed. */
            pr_err_v("Failed to create chardev for demo item [%u], err = %ld\n", i, PTR_ERR(demo_item->device));
            kfree(demo_item);
            break;
        }
        atomic_set(&demo_item->open_count, 0);
        /* Initialization of other fields if any. */
        pr_notice_v("Device[%s%u]: priv_addr = 0x%p\n",
            DEFAULT_GRP_ITEM_BASENAME, MINOR(demo_item->device->devt), demo_item);
    }

    if (0 == i)
    {
        if (!ret)
            ret = -ENOMEM;

        goto lbl_destroy_group;
    }

    goto lbl_init_end;

lbl_destroy_group:

    CHRDEV_GRP_DESTROY(kfree);

lbl_init_end:

    return ret;
}

static int __user_group_demo_init(void)
{
    uint i;
    int ret = 0;

    if (IS_ERR(s_user_group = chardev_group_create(KBUILD_MODNAME, mibase, ncount, &S_FILE_OPS)))
    {
        ret = PTR_ERR(s_user_group);
        goto lbl_init_end;
    }

    for (i = 0; i < ncount; ++i) /* NOTE: There's no loop in xxx_probe(). */
    {
        demo_struct_t *demo_item = (demo_struct_t *)kzalloc(sizeof(demo_struct_t), GFP_KERNEL);

        if (NULL == demo_item)
        {
            /* ret = -ENOMEM; */ /* Don't modify ret if partial failure is allowed. */
            pr_err_v("Failed to allocate memory for demo item [%u]\n", i);
            break;
        }

        if (IS_ERR(demo_item->device = chardev_group_make_item(s_user_group, USER_GRP_ITEM_BASENAME, demo_item)))
        {
            /* ret = PTR_ERR(demo_item->device); */ /* Don't modify ret if partial failure is allowed. */
            pr_err_v("Failed to create chardev for demo item [%u], err = %ld\n", i, PTR_ERR(demo_item->device));
            kfree(demo_item);
            break;
        }
        atomic_set(&demo_item->open_count, 0);
        /* Initialization of other fields if any. */
        pr_notice_v("Created device[%s%u]: priv_addr = 0x%p\n",
            USER_GRP_ITEM_BASENAME, MINOR(demo_item->device->devt), demo_item);
    }

    if (0 == i)
    {
        if (!ret)
            ret = -ENOMEM;

        goto lbl_destroy_group;
    }

    goto lbl_init_end;

lbl_destroy_group:

    chardev_group_destroy(&s_user_group, kfree);

lbl_init_end:

    return ret;
}

static __init int chardev_group_demo_init(void)
{
    int ret = (DEFAULT_GRP_VAR_FLAG == gvar) ? __default_group_demo_init() : __user_group_demo_init();

    if (!ret)
    {
        const char *basename = (DEFAULT_GRP_VAR_FLAG == gvar) ? DEFAULT_GRP_ITEM_BASENAME : USER_GRP_ITEM_BASENAME;

        pr_notice_v("Module initialized.\n");
        pr_notice_v("Run commands below to check whether the driver and its underlying functions work correctly:\n");
        pr_notice_v("$ ls -l /dev/%s*\n", basename);
        pr_notice_v("$ cat /dev/%s*\n", basename);
    }

    return ret;
}

static __exit void chardev_group_demo_exit(void)
{
    if (DEFAULT_GRP_VAR_FLAG == gvar)
        CHRDEV_GRP_DESTROY(kfree);
    else
        chardev_group_destroy(&s_user_group, kfree);

    pr_notice_v("Module exited.\n");
}

module_init(chardev_group_demo_init);
module_exit(chardev_group_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Man Hung-Coeng <udc577@126.com>");

#endif /* #ifdef TEST */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-11-03, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 *
 * >>> 2023-11-05, Man Hung-Coeng <udc577@126.com>:
 *  01. Transfer the definition of struct chardev_group
 *      from header file to this one.
 *  02. Add free_privdata callback pointer to parameter list of
 *      chardev_group_destroy() and chardev_group_unmake_item().
 *  03. Add usage demo.
 *
 * >>> 2023-12-16, Man Hung-Coeng <udc577@126.com>:
 *  01. Add property accessor functions.
 *
 * >>> 2024-06-16, Man Hung-Coeng <udc577@126.com>:
 *  01. Fix the compilation error of class_create() on kernel 6.4.0 and above.
 */

