/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Supplements to device class in linux mainline.
 *
 * Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
*/

#ifndef __DEVCLASS_SUPPLEMENTS_H__
#define __DEVCLASS_SUPPLEMENTS_H__

#ifdef __cplusplus
extern "C" {
#endif

struct class;
struct class_attribute;

int class_create_files(struct class *cls, const struct class_attribute *attrs);

void class_remove_files(struct class *cls, const struct class_attribute *attrs);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __DEVCLASS_SUPPLEMENTS_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-12-17, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 */

