/*
 * Simple APIs for Windows .ini file manipulation.
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

#define INI_NODE_BLANK_LINE     '\0'
#define INI_NODE_COMMENT        'c'
#define INI_NODE_SECTION        's'
#define INI_NODE_ITEM           'i'

struct ini_node_t;
typedef struct ini_node_t ini_node_t;

struct ini_cfg_t;
typedef struct ini_cfg_t ini_cfg_t;

typedef struct ini_summary_t
{
    int error_code;
    int success_lines;
    int section_lines;
    int comment_lines;
    int blank_lines;
} ini_summary_t;

typedef int (*ini_traverval_callback_t)(const char *parent_sec_name, ini_node_t *cur_node, void *extra);

const char* ini_error(int error_code);

void ini_set_newline(const char* const newline); /* \n for *nix, \r\n for Windows. */

void ini_set_item_indent_width(size_t width);

char ini_node_type(const ini_node_t *node);

ini_summary_t ini_parse(const FILE *stream, ini_cfg_t *cfg);

ini_summary_t ini_dump(const ini_cfg_t *cfg, FILE *stream);

void ini_destroy(ini_cfg_t *cfg);

ini_summary_t ini_traverse_all_nodes(ini_cfg_t *cfg, ini_traverval_callback_t cb, void *cb_arg);

ini_summary_t ini_traverse_nodes_of(ini_node_t *sec, ini_traverval_callback_t cb, void *cb_arg);

/*
 * ================
 *  SECTION
 * ================
 */

ini_node_t* ini_section_find(const ini_cfg_t *cfg, const char *name);

const char* ini_section_get_name(const ini_node_t *sec);

int ini_section_rename(const char *name, size_t name_len, ini_node_t *sec);

/*
 * ================
 *  ITEM
 * ================
 */


ini_node_t* ini_item_find(const ini_node_t *sec, const char *key);

const char* ini_item_get_key(const ini_node_t *item);

int ini_item_set_key(const char *key, size_t key_len, ini_node_t *item);

const char* ini_item_get_value(const ini_node_t *item);

int ini_item_set_value(const char *val, size_t val_len, ini_node_t *item);

/*
 * ================
 *  COMMENT
 * ================
 */


const char* ini_comment_get(const ini_node_t *node);

int ini_comment_set(const char *comment, size_t comment_len, ini_node_t *node);

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
 *
 * >>> 2021-12-17, Man Hung-Coeng:
 *  01. Enrich interfaces, and hide unnecessary structs.
 */

