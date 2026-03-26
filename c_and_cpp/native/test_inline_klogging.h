/* SPDX-License-Identifier: GPL-2.0 */

/*
 * A demo showing how to use klogging interfaces in an inline function
 * within a header file, which is not recommended, though.
 * See also: ../../makefiles/linux_driver.mk
 *
 * Copyright (c) 2023-2026 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
*/

#ifndef __TEST_INLINE_KLOGGING_H__
#define __TEST_INLINE_KLOGGING_H__

#include <linux/printk.h>

#include "klogging.h"

void test_inline_klogging(void)
{
    pr_info_v("Check the output of __FILE__, __LINE__ and __func__ of this message yourself.\n");
}

#define TEST_KLOGGING_MACRO()        do { \
    pr_notice_v("Check the output of __FILE__, __LINE__ and __func__ of this message yourself.\n"); \
} while (0)

#endif /* #ifndef __TEST_INLINE_KLOGGING_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-11-07, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 *
 * >>> 2026-03-26, Man Hung-Coeng <udc577@126.com>:
 *  01. Update the directory of linux_driver.mk in comment.
 */

