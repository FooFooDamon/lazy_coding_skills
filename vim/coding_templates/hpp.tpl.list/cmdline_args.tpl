/* SPDX-License-Identifier: Apache-2.0 */

/*
 * APIs for parsing and validating command-line arguments.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#ifndef __${HEADER_GUARD}_HPP__
#define __${HEADER_GUARD}_HPP__

#include <string>
#include <vector>

typedef struct cmd_args
{
    std::vector<std::string> orphan_args;
    std::string biz;
    std::string config_file;
#ifdef HAS_LOGGER
    std::string log_file;
    std::string log_level;
#else
    bool verbose;
    bool debug;
#endif
    // FIXME: Add more fields according to your need, and delete this comment line.
} cmd_args_t;

cmd_args_t parse_cmdline(int argc, char **argv);

void assert_parsed_args(const cmd_args_t &args);

#endif /* #ifndef __${HEADER_GUARD}_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
