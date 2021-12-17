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

#include "simple_ini_config.h"

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct ini_section_t
{
    char *name;
    struct ini_node_t *sub;
} ini_section_t;

typedef struct ini_item_t
{
    char *key;
    char *val;
} ini_item_t;

enum
{
    INI_ERR_UNKNOWN = 1
    , INI_ERR_MEM_ALLOC
    , INI_ERR_STRING_TOO_LONG
    , INI_ERR_NOT_SECTION_NODE
    , INI_ERR_NOT_ITEM_NODE
    , INI_ERR_ORPHAN_ITEM
    , INI_ERR_IO
    , INI_ERR_COMMENT_NOT_ALLOWED
};

static const char* const S_ERRORS[] = {
    "Unknown error"
    , "Failed to allocate memory"
    , "String too long"
    , "Not a section node"
    , "Not an item node"
    , "Orphan item"
    , "I/O error"
    , "Comment not allowed"
};

const char* ini_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    return S_ERRORS[-error_code - 1];
}

static const char *s_ini_newline = "\n";

void ini_set_newline(const char* const newline)
{
    s_ini_newline = newline;
}

static char *s_indent_spaces = NULL;

void ini_set_item_indent_width(size_t width)
{
    if (NULL != s_indent_spaces)
    {
        free(s_indent_spaces);
        s_indent_spaces = NULL;
    }

    if (0 == width || width > 16)
        return;

    s_indent_spaces = malloc(width + 1);
    if (NULL != s_indent_spaces)
    {
        size_t i = 0;

        for (; i < width; ++i)
        {
            s_indent_spaces[i] = ' ';
        }
        s_indent_spaces[width] = '\0';
    }
}

char ini_node_type(const ini_node_t *node)
{
    return node->type;
}

ini_summary_t ini_parse(const FILE *stream, ini_cfg_t *cfg)
{
    ini_summary_t summary = { 0 };

    summary.error_code = INI_ERR_UNKNOWN;

    /* TODO */

    return summary;
}

static ini_summary_t __traverse_nodes_of(ini_node_t *sec, int should_free_memory,
    ini_traverval_callback_t cb, void *cb_arg)
{
    ini_summary_t summary = { 0 };
    ini_node_t *node = (NULL == sec || INI_NODE_SECTION != sec->type) ? NULL : ((ini_section_t *)sec->detail)->sub;
    const char *sec_name = (NULL == sec) ? NULL : ((ini_section_t *)sec)->name;

    while (NULL != node)
    {
        char node_type = node->type;
        ini_node_t *node_ptr = node;
        ini_item_t *item = NULL;

        node = node->next;

        if (NULL != cb)
        {
            int err = cb(sec_name, node_ptr, cb_arg);

            if (err < 0)
            {
                summary.error_code = err;

                return summary;
            }
            else
            {
                int *couter = (INI_NODE_ITEM == node_type) ? NULL
                    : ((INI_NODE_COMMENT == node_type) ? &summary.comment_lines : &summary.blank_lines);

                summary.success_lines += 1;
                if (NULL != couter)
                    *couter += 1;
            }
        }

        /* NOTE: Do the assignment here in case that memeory reallocation happens within cb(). */
        item = (ini_item_t *)node_ptr->detail;

        if (should_free_memory)
        {
            free(node_ptr);

            if (INI_NODE_ITEM == node_type)
            {
                free(item->key);
                free(item->val);
                free(item);
            }

            free(item);
        }
    } /* while (NULL != node) */

    return summary;
}

static ini_summary_t __traverse_all_nodes(ini_cfg_t *cfg, int should_free_memory,
    ini_traverval_callback_t cb, void *cb_arg)
{
    ini_summary_t summary = { 0 };
    ini_node_t *node = cfg->head;

    while (NULL != node)
    {
        char node_type = node->type;
        ini_node_t *node_ptr = node;

        node = node->next;

        if (NULL != cb)
        {
            int err = cb(NULL, node_ptr, cb_arg);

            if (err < 0)
            {
                summary.error_code = err;

                return summary;
            }
            else
            {
                int *couter = (INI_NODE_SECTION == node_type) ? &summary.section_lines
                    : ((INI_NODE_COMMENT == node_type) ? &summary.comment_lines : &summary.blank_lines);

                summary.success_lines += 1;
                *couter += 1;
            }
        }

        if (INI_NODE_SECTION == node_type)
        {
            ini_summary_t inner_summary = __traverse_nodes_of(node_ptr, should_free_memory, cb, cb_arg);

            summary.success_lines += inner_summary.success_lines;
            summary.section_lines += inner_summary.section_lines;
            summary.comment_lines += inner_summary.comment_lines;
            summary.blank_lines += inner_summary.blank_lines;

            if (inner_summary.error_code < 0)
            {
                summary.error_code = inner_summary.error_code;

                return summary;
            }
        }

        if (should_free_memory)
        {
            if (INI_NODE_SECTION == node_type)
            {
                ini_section_t *sec = (ini_section_t *)node_ptr->detail;

                free(sec->name);
                free(sec);
            }
            free(node_ptr);
        }
    } /* while (NULL != node) */

    return summary;
}

static int __dump_node(const char *parent_sec_name, ini_node_t *cur_node, void *stream)
{
    int bytes_written = -INI_ERR_IO;

    switch (cur_node->type)
    {
    case INI_NODE_BLANK_LINE:
        bytes_written = fputs(s_ini_newline, stream);
        break;

    case INI_NODE_COMMENT:
        bytes_written = fputs(cur_node->detail, stream);
        if (bytes_written <= 0)
            break;

    case INI_NODE_SECTION:
        bytes_written = fputc('[', stream);
        if (bytes_written > 0)
            bytes_written += fputs(((ini_section_t *)cur_node->detail)->name, stream);
        if (bytes_written > 0)
            bytes_written += fputc(']', stream);
        if (bytes_written <= 0)
            break;

    case INI_NODE_ITEM:
        if (NULL == s_indent_spaces)
            bytes_written = 0;
        else
            bytes_written = fputs(s_indent_spaces, stream);
        if (bytes_written >= 0)
            bytes_written = fputs(((ini_item_t *)cur_node)->key, stream);
        if (bytes_written > 0)
            bytes_written = fputs(" =", stream);
        if (bytes_written > 0)
            bytes_written = fputs(((ini_item_t *)cur_node)->val, stream);
        if (bytes_written <= 0)
            break;

    default:
        bytes_written += fputs(s_ini_newline, stream);
        break;
    }

    return bytes_written;
}

ini_summary_t ini_dump(const ini_cfg_t *cfg, FILE *stream)
{
    return __traverse_all_nodes((ini_cfg_t *)cfg, 0, __dump_node, stream);
}

void ini_destroy(ini_cfg_t *cfg)
{
    if (__traverse_all_nodes(cfg, 1, NULL, NULL).success_lines > 0)
        cfg->head = NULL;
}

ini_summary_t ini_traverse_all_nodes(ini_cfg_t *cfg, ini_traverval_callback_t cb, void *cb_arg)
{
    return __traverse_all_nodes(cfg, 0, __dump_node, cb_arg);
}

ini_summary_t ini_traverse_nodes_of(ini_node_t *sec, ini_traverval_callback_t cb, void *cb_arg)
{
    return __traverse_nodes_of(sec, 0, __dump_node, cb_arg);
}

/*
 * ================
 *  SECTION
 * ================
 */

ini_node_t* ini_section_find(const ini_cfg_t *cfg, const char *name)
{
    ini_node_t *node = cfg->head;

    for (; NULL != node; node = node->next)
    {
        if (INI_NODE_SECTION == node->type &&
            0 == strncmp(((ini_section_t *)node->detail)->name, name, INI_LINE_SIZE_MAX))
        {
            return node;
        }
    }

    return NULL;
}

const char* ini_section_get_name(const ini_node_t *sec)
{
    return (INI_NODE_SECTION == sec->type) ? (((ini_section_t *)sec->detail)->name) : NULL;
}

int ini_section_rename(const char *name, size_t name_len, ini_node_t *sec)
{
    ini_section_t *detail = (INI_NODE_SECTION == sec->type) ? ((ini_section_t *)sec->detail) : NULL;
    int is_too_long = (name_len > INI_LINE_SIZE_MAX - 1);
    char *new_name = (NULL == detail || is_too_long) ? NULL : malloc(name_len + 1);

    if (NULL == detail)
        return INI_ERR_NOT_SECTION_NODE;

    if (is_too_long)
        return INI_ERR_STRING_TOO_LONG;

    if (NULL == new_name)
        return INI_ERR_MEM_ALLOC;

    strncpy(new_name, name, name_len);
    new_name[name_len] = '\0';

    free(detail->name);
    detail->name = new_name;

    return name_len;
}

/*
 * ================
 *  ITEM
 * ================
 */


ini_node_t* ini_item_find(const ini_node_t *sec, const char *key)
{
    ini_node_t *node = (INI_NODE_SECTION != sec->type) ? (((ini_section_t *)sec)->sub) : NULL;

    for (; NULL != node; node = node->next)
    {
        if (INI_NODE_ITEM == node->type &&
            0 == strncmp(((ini_item_t *)node->detail)->key, key, INI_LINE_SIZE_MAX))
        {
            return node;
        }
    }

    return NULL;
}

const char* ini_item_get_key(const ini_node_t *item)
{
    return (INI_NODE_ITEM == item->type) ? (((ini_item_t *)item->detail)->key) : NULL;
}

int ini_item_set_key(const char *key, size_t key_len, ini_node_t *item)
{
    ini_item_t *detail = (INI_NODE_ITEM == item->type) ? ((ini_item_t *)item->detail) : NULL;
    int is_too_long = (key_len > INI_LINE_SIZE_MAX - 1);
    char *new_key = (NULL == detail || is_too_long) ? NULL : malloc(key_len + 1);

    if (NULL == detail)
        return INI_ERR_NOT_ITEM_NODE;

    if (is_too_long)
        return INI_ERR_STRING_TOO_LONG;

    if (NULL == new_key)
        return INI_ERR_MEM_ALLOC;

    strncpy(new_key, key, key_len);
    new_key[key_len] = '\0';

    free(detail->key);
    detail->key = new_key;

    return key_len;
}

const char* ini_item_get_value(const ini_node_t *item)
{
    return (INI_NODE_ITEM == item->type) ? (((ini_item_t *)item->detail)->val) : NULL;
}

int ini_item_set_value(const char *val, size_t val_len, ini_node_t *item)
{
    ini_item_t *detail = (INI_NODE_ITEM == item->type) ? ((ini_item_t *)item->detail) : NULL;
    int is_too_long = (val_len > INI_LINE_SIZE_MAX - 1);
    char *new_value = (NULL == detail || is_too_long) ? NULL : malloc(val_len + 1);

    if (NULL == detail)
        return INI_ERR_NOT_ITEM_NODE;

    if (is_too_long)
        return INI_ERR_STRING_TOO_LONG;

    if (NULL == new_value)
        return INI_ERR_MEM_ALLOC;

    strncpy(new_value, val, val_len);
    new_value[val_len] = '\0';

    free(detail->val);
    detail->val = new_value;

    return val_len;
}

/*
 * ================
 *  COMMENT
 * ================
 */


const char* ini_comment_get(const ini_node_t *node)
{
    return (INI_NODE_COMMENT == node->type) ? (node->detail) : NULL;
}

int ini_comment_set(const char *comment, size_t comment_len, ini_node_t *node)
{
    int is_too_long = (comment_len > INI_LINE_SIZE_MAX - 1);
    int allowed = (INI_NODE_COMMENT == node->type || INI_NODE_BLANK_LINE == node->type);
    char *new_comment = (is_too_long || !allowed) ? NULL : malloc(comment_len + 1);

    if (is_too_long)
        return INI_ERR_STRING_TOO_LONG;

    if (!allowed)
        return INI_ERR_COMMENT_NOT_ALLOWED;

    if (NULL == new_comment)
        return INI_ERR_MEM_ALLOC;

    strncpy(new_comment, comment, comment_len);
    new_comment[comment_len] = '\0';

    if (NULL != node->detail)
        free(node->detail);
    node->detail = new_comment;
    if (INI_NODE_BLANK_LINE == node->type)
        node->type = INI_NODE_COMMENT;

    return comment_len;
}

#ifdef TEST

int main(int argc, char **argv)
{
    printf("TODO ...\n");

    return 0;
}

#endif /* #ifdef TEST */

#ifdef __cplusplus
}
#endif

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2021-12-15, Man Hung-Coeng:
 *  01. Create.
 *
 * >>> 2021-12-17, Man Hung-Coeng:
 *  01. Enrich interfaces, and implement them except for ini_parse().
 */

