/*
 * APIs for Windows .ini file manipulation.
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

#ifndef __INI_FILE_H__
#define __INI_FILE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INI_LINE_SIZE_MAX
#define INI_LINE_SIZE_MAX       4095
#if INI_LINE_SIZE_MAX < 63
#error INI_LINE_SIZE_MAX is too small!
#endif
#endif

#ifndef INI_KEY_SIZE_MAX
#define INI_KEY_SIZE_MAX        INI_LINE_SIZE_MAX / 6
#if INI_KEY_SIZE_MAX < 10
#error INI_KEY_SIZE_MAX is too small!
#endif
#endif

#ifndef INI_INDENT_WIDTH_MAX
#define INI_INDENT_WIDTH_MAX    16
#endif

#define INI_NODE_BLANK_LINE     '\0'
#define INI_NODE_COMMENT        'c'
#define INI_NODE_SECTION        's'
#define INI_NODE_ITEM           'i'

struct ini_node_t;
typedef struct ini_node_t ini_node_t;

struct ini_doc_t;
typedef struct ini_doc_t ini_doc_t;

typedef struct ini_summary_t
{
    int error_code;
    int success_lines;
    int section_lines;
    int comment_lines;
    int blank_lines;
} ini_summary_t;

typedef int (*ini_traverval_callback_t)(const char *sec_name, ini_node_t *cur_node, void *callback_arg);

const char* ini_error(int error_code);

void ini_set_newline(const char* const newline); /* \n for *nix, \r\n for Windows. */

void ini_set_item_indent_width(size_t width);

char ini_node_type(const ini_node_t *node);

ini_doc_t* ini_parse_from_stream(FILE *stream, int strip_blanks/* = 0 or 1, for item value only */,
    ini_summary_t *nullable_summary/* = NULL if failure reason not cared*/);

ini_doc_t* ini_parse_from_buffer(char *buf, size_t buf_len, int strip_blanks/* = 0 or 1, for item value only */,
    ini_summary_t *nullable_summary/* = NULL if failure reason not cared*/);

ini_summary_t ini_dump_to_stream(const ini_doc_t *doc, FILE *stream);

ini_summary_t ini_dump_to_buffer(const ini_doc_t *doc, char **buf, size_t *buf_len, int allow_resizing);

void ini_destroy(ini_doc_t **doc);

/*
 * ================
 *  TRAVERSAL
 * ================
 */

ini_summary_t ini_traverse_all_nodes(ini_doc_t *doc, ini_traverval_callback_t cb, void *cb_arg);

ini_summary_t ini_traverse_all_sections(ini_doc_t *doc, ini_traverval_callback_t cb, void *cb_arg);

ini_summary_t ini_traverse_nodes_of(ini_node_t *sec, ini_traverval_callback_t cb, void *cb_arg);

/*
 * ================
 *  SECTION
 * ================
 */

ini_node_t* ini_section_find(const ini_doc_t *doc, const char *name, size_t name_len/* = 0 if auto calculated later */);

int ini_section_is_repeated(const ini_doc_t *doc, const char *name, size_t name_len/* = 0 if auto calculated later */);

const char* ini_section_get_name(const ini_node_t *sec);

int ini_section_rename(const char *name, size_t name_len, ini_node_t *sec);

int ini_section_add(const char *name, size_t name_len, ini_doc_t *doc);

int ini_section_remove(const char *name, size_t name_len/* = 0 if auto calculated later */, ini_doc_t *doc);

/*
 * ================
 *  ITEM
 * ================
 */

ini_node_t* ini_item_find(const ini_node_t *sec, const char *key, size_t key_len/* = 0 if auto calculated later */);

int ini_item_is_repeated(const ini_node_t *sec, const char *key, size_t key_len/* = 0 if auto calculated later */);

const char* ini_item_get_key(const ini_node_t *item);

int ini_item_set_key(const char *key, size_t key_len, ini_node_t *item);

const char* ini_item_get_value(const ini_node_t *item);

int ini_item_set_value(const char *val, size_t val_len, ini_node_t *item);

int ini_item_add(const char *key, size_t key_len, const char *val, size_t val_len, ini_node_t *sec);

int ini_item_remove(const char *key, size_t key_len/* = 0 if auto calculated later */, ini_node_t *sec);

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

#endif /* #ifndef __INI_FILE_H__ */

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
 *
 * >>> 2021-12-20, Man Hung-Coeng:
 *  01. Fix errors in the existing functions.
 *  02. Implement ini_parse() and rename it to ini_parse_from_stream();
 *      rename ini_dump() to ini_dump_to_stream().
 *  03. Add ini_{section,item}_is_repeated(), ini_traverse_all_sections(),
 *      ini_dump_to_buffer() (not implemented),
 *      ini_parse_from_buffer() (not implemented),
 *      and ini_{section,item}_{add,remove}() (not implemented).
 *
 * >>> 2021-12-21, Man Hung-Coeng:
 *  01. Remove unnecessary parameters from ini_{section,item}_remove().
 *  02. Change return type of ini_parse_from_{stream,buffer}() to ini_cfg_t*
 *      in order to solve the "incomplete type" error of ini_cfg_t
 *      when the two functions are called outside this file.
 *  03. Change parameter type of ini_destroy() from ini_cfg_t* to ini_cfg_t**.
 *
 * >>> 2021-12-22, Man Hung-Coeng:
 *  01. Implement ini_parse_from_buffer() and ini_dump_to_buffer().
 *  02. Re-add *_len parameter to some functions for future optimization.
 *
 * >>> 2021-12-31, Man Hung-Coeng:
 *  01. Rename file simple_ini_config.{c,h} to ini_file.{c,h}.
 *  02. Rename structure ini_cfg_t to ini_doc_t.
 */

