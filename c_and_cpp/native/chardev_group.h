/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Chardev wrappers organized in groups.
 *
 * Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
*/

#ifndef __CHARDEV_GROUP_H__
#define __CHARDEV_GROUP_H__

#include <linux/cdev.h>

typedef struct chardev_group
{
    struct device **items;
    struct class *class;
    struct cdev cdev;
} chardev_group_t;

chardev_group_t** __chardev_group_default_pptr(void);
/* THIS_CHRDEV_GRP is the default group, just like THIS_MODULE. */
#define THIS_CHRDEV_GRP                                 (*__chardev_group_default_pptr())

chardev_group_t* chardev_group_create(const char *name, int baseminor, int max_items,
    const struct file_operations *fops);
#define CHRDEV_GRP_CREATE(name, minor, count, fops)     (THIS_CHRDEV_GRP = chardev_group_create(name, minor, count, fops))

void chardev_group_destroy(chardev_group_t **group);
#define CHRDEV_GRP_DESTROY()                            chardev_group_destroy(__chardev_group_default_pptr())

struct device* chardev_group_make_item(chardev_group_t *group, const char *basename, void *private_data);
#define CHRDEV_GRP_MAKE_ITEM(name, privdata)            chardev_group_make_item(THIS_CHRDEV_GRP, name, privdata)

void chardev_group_unmake_item(chardev_group_t *group, struct device *item);
#define CHRDEV_GRP_UNMAKE_ITEM(item)                    chardev_group_unmake_item(THIS_CHRDEV_GRP, item)

struct device* chardev_group_find_item(dev_t dev_id, const chardev_group_t *group);
#define CHRDEV_GRP_FIND_ITEM_BY_INODE(inode_ptr)        chardev_group_find_item((inode_ptr)->i_rdev, THIS_CHRDEV_GRP)

void* chardev_group_find_item_private_data(dev_t dev_id, const chardev_group_t *group);
#define CHRDEV_GRP_FIND_ITEM_PRIVDATA_BY_INODE(inode_ptr)   chardev_group_find_item_private_data((inode_ptr)->i_rdev, THIS_CHRDEV_GRP)

#endif /* #ifndef __CHARDEV_GROUP_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-11-03, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 */

