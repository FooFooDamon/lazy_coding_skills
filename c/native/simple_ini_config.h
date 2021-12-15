/*
 * Simple APIs for Windows .ini file parsing.
 *
 * Copyright (c) 2021 Man Hung-Coeng <udc577@126.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef __SIMPLE_INI_CONFIG_H__
#define __SIMPLE_INI_CONFIG_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INI_LINE_SIZE_MAX
#define INI_LINE_SIZE_MAX       4096
#endif

#define INI_NODE_SECTION        1
#define INI_NODE_ITEM           2
#define INI_NODE_COMMENT        3
#define INI_NODE_BLANK_LINE     4

extern char const *g_ini_newline;

#define INI_SET_NEWLINE(nlstr)  g_ini_newline = nlstr

struct ini_node_t
{
    char type;
    void *detail;
    struct ini_node_t *next;
};

struct ini_cfg_t
{
    struct ini_node_t *head;
};

struct ini_section_t
{
    char *name;
    char *comment;
    void *sub;
};

struct ini_item_t
{
    char *key;
    char *val;
    char *comment;
};

int ini_parse(const FILE *stream, struct ini_cfg_t *cfg, char *desc);

int ini_dump(const struct ini_cfg_t *cfg, FILE *stream);

void ini_destroy(struct ini_cfg_t *cfg);

struct ini_node_t* ini_find_section(struct ini_cfg_t *cfg, const char *name);

struct ini_node_t* ini_find_item(struct ini_section_t *section, const char *key);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __SIMPLE_INI_CONFIG_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2021-12-15, Man Hung-Coeng:
 *  01. Create.
 */

