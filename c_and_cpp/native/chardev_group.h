/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Chardev wrappers organized in groups.
 *
 * Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
*/

#ifndef __CHARDEV_GROUP_H__
#define __CHARDEV_GROUP_H__

#include <linux/types.h> /* For dev_t. */

struct chardev_group;
struct device;
struct file_operations;

struct chardev_group** __chardev_group_default_pptr(void);
/* THIS_CHRDEV_GRP is the default group, just like THIS_MODULE as the default module. */
#define THIS_CHRDEV_GRP                                 (*__chardev_group_default_pptr())

/* Used in xxx_init() once. */
struct chardev_group* chardev_group_create(const char *name, unsigned int baseminor, unsigned int max_items,
    const struct file_operations *fops);
#define CHRDEV_GRP_CREATE(name, minor, cnt, fops)       (THIS_CHRDEV_GRP = chardev_group_create(name, minor, cnt, fops))

/*
 * Used in xxx_exit() once.
 * Set free_privdata to NULL if you want to destroy each item and its private data yourself.
 */
void chardev_group_destroy(struct chardev_group **group, void (*free_privdata)(const void *));
#define CHRDEV_GRP_DESTROY(free_func)                   chardev_group_destroy(__chardev_group_default_pptr(), free_func)

/* Used in xxx_{probe,init}() one (for a single device node) or multiple times (for multiple device nodes). */
struct device* chardev_group_make_item(struct chardev_group *group, const char *basename, void *private_data);
#define CHRDEV_GRP_MAKE_ITEM(name, privdata)            chardev_group_make_item(THIS_CHRDEV_GRP, name, privdata)

/*
 * Like the ones above, except in xxx_{disconnect,remove}().
 * Set free_privdata to NULL if you want to destroy item's private data yourself.
 */
void chardev_group_unmake_item(struct chardev_group *group, struct device *item, void (*free_privdata)(const void *));
#define CHRDEV_GRP_UNMAKE_ITEM(item, free_func)         chardev_group_unmake_item(THIS_CHRDEV_GRP, item, free_func)

struct device* chardev_group_find_item(dev_t dev_id, const struct chardev_group *group);
#define CHRDEV_GRP_FIND_ITEM_BY_INODE(inode_ptr)        chardev_group_find_item((inode_ptr)->i_rdev, THIS_CHRDEV_GRP)

/* Used in xxx_open() to get the private pointer previously set and assign it to file->private_data. */
void* chardev_group_find_item_private_data(dev_t dev_id, const struct chardev_group *group);
#define CHRDEV_GRP_FIND_ITEM_PRIVDATA_BY_INODE(inode_ptr)   chardev_group_find_item_private_data((inode_ptr)->i_rdev, THIS_CHRDEV_GRP)

#endif /* #ifndef __CHARDEV_GROUP_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-11-03, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 *
 * >>> 2023-11-05, Man Hung-Coeng <udc577@126.com>:
 *  01. Hide the definition of struct chardev_group
 *      to make this header file more lightweight.
 *  02. Add free_privdata callback pointer to parameter list of
 *      chardev_group_destroy() and chardev_group_unmake_item().
 *  03. Add usage hint comments.
 */

