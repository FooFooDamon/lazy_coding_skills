/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Common biz declarations and definitions.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#ifndef __${HEADER_GUARD}_HPP__
#define __${HEADER_GUARD}_HPP__

#include <stdio.h>

// FIXME: Include other header files here according to your need, and delete this comment line.

struct cmd_args;
struct conf_file;

typedef struct biz_context
{
    int argc;
    char **argv;
    struct cmd_args *cmd_args;
    struct conf_file *conf;
    // FIXME: Add more fields according to your need, and delete this comment line.
} biz_context_t;

#define BIZ_FUN_ARG_LIST                biz_context_t &ctx

#define DECLARE_BIZ_FUN(name)           int name(BIZ_FUN_ARG_LIST)
#define BIZ_FUN(name)                   name

typedef int (*biz_func_t)(BIZ_FUN_ARG_LIST);

#ifndef TODO
#define TODO()                          fprintf(stderr, __FILE__ ":%d %s(): TODO ...\n", __LINE__, __func__)
#endif

// FIXME: Add more stuff according to your need, and delete this comment line.

#endif /* #ifndef __${HEADER_GUARD}_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
