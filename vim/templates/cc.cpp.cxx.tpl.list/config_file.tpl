// SPDX-License-Identifier: Apache-2.0

/*
 * APIs for loading and unloading configuration file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#include "${BASENAME}.hpp"

#include <stdio.h>

int load_config_file(const char *path, conf_file_t &result)
{
#ifdef HAS_CONFIG_FILE
    fprintf(stderr, __FILE__ ":%d %s(): TODO ...\n", __LINE__, __func__);
#endif

    return 0;
}

void unload_config_file(conf_file_t &result)
{
    fprintf(stderr, __FILE__ ":%d %s(): TODO ...\n", __LINE__, __func__);
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
