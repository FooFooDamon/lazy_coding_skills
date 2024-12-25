// SPDX-License-Identifier: Apache-2.0

/*
 * Entry point of this project.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

//#include "${BASENAME}.hpp"

#include <stdlib.h>
#include <stdio.h>

#include "versions.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "cmdline_args.hpp"
#include "config_file.hpp"
#include "biz_common.hpp"

int logger_init(const cmd_args_t &args, const conf_file_t &conf)
{
#ifdef HAS_LOGGER
    TODO();
#endif
    return 0;
}

void logger_finalize(void)
{
#ifdef HAS_LOGGER
    TODO();
#endif
}

int register_signals(const cmd_args_t &args, const conf_file_t &conf)
{
#ifdef NEED_OS_SIGNALS
    TODO();
#endif
    return 0;
}

static DECLARE_BIZ_FUN(normal_biz)
{
    TODO();

    return EXIT_SUCCESS;
}

static DECLARE_BIZ_FUN(test_biz)
{
    TODO();

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    cmd_args_t parsed_args = parse_cmdline(argc, argv);
    conf_file_t conf;
    biz_context_t ctx = {
        argc, argv, &parsed_args, &conf,
    };
    std::map<std::string, biz_func_t> biz_handlers = {
        { "normal", BIZ_FUN(normal_biz) },
        { "test", BIZ_FUN(test_biz) },
    };
    biz_func_t biz_func = nullptr;
    int ret;

    assert_parsed_args(parsed_args);

    if (nullptr == (biz_func = biz_handlers[parsed_args.biz]))
    {
        fprintf(stderr, "*** Biz[%s] is not supported yet!\n", parsed_args.biz.c_str());
        return ENOTSUP;
    }

    if ((ret = load_config_file(parsed_args.config_file.c_str(), conf)) < 0)
        return -ret;

    if ((ret = logger_init(parsed_args, conf)) < 0)
        goto lbl_unload_conf;

    if ((ret = register_signals(parsed_args, conf)) < 0)
        goto lbl_finalize_log;

    ret = biz_func(ctx);

lbl_finalize_log:
    logger_finalize();

lbl_unload_conf:
    unload_config_file(conf);

    return abs(ret);
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
