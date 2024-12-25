/* SPDX-License-Identifier: Apache-2.0 */

/*
 * APIs for loading and unloading configuration file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#ifndef __${HEADER_GUARD}_HPP__
#define __${HEADER_GUARD}_HPP__

#include <string>

typedef struct conf_file
{
    std::string path;
    // FIXME: Add more fields according to your need, and delete this comment line.
} conf_file_t;

int load_config_file(const char *path, conf_file_t &result);

void unload_config_file(conf_file_t &result);

#endif /* #ifndef __${HEADER_GUARD}_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
