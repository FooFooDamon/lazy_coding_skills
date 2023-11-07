// SPDX-License-Identifier: GPL-2.0

/*
 * A demo showing how to use klogging interfaces in an inline function
 * within a header file, which is not recommended, though.
 * See also: ../../makefile/linux_driver.mk
 *
 * Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
*/

#ifdef TEST

#undef __SRC__
#define __SRC__ "./test_inline_klogging.h"
#include "test_inline_klogging.h"
#undef __SRC__
#define __SRC__ "test_inline_klogging.c"

#include <linux/module.h>

static __init int test_inline_klogging_init(void)
{
    test_inline_klogging();
    TEST_KLOGGING_MACRO();
    pr_notice_v("Module initialized.\n");

    return 0;
}

static __exit void test_inline_klogging_exit(void)
{
    pr_notice_v("Module exited.\n");
}

module_init(test_inline_klogging_init);
module_exit(test_inline_klogging_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Man Hung-Coeng <udc577@126.com>");

#endif /* #ifdef TEST */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-11-07, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 */

