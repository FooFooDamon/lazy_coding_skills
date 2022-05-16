/*
 * APIs for Windows .ini file manipulation.
 *
 * Copyright (c) 2021-2022 Man Hung-Coeng <udc577@126.com>
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

#include "ini_file.h"

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

struct ini_doc_t
{
    struct ini_node_t *preamble;
    struct ini_node_t *section;
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
    , INI_ERR_NOT_IMPLEMENTED
    , INI_ERR_MEM_ALLOC
    , INI_ERR_STRING_TOO_LONG
    , INI_ERR_UNKNOWN_NODE_TYPE
    , INI_ERR_NOT_SECTION_NODE
    , INI_ERR_NOT_ITEM_NODE
    , INI_ERR_ORPHAN_ITEM
    , INI_ERR_IO
    , INI_ERR_COMMENT_NOT_ALLOWED
    , INI_ERR_RESIDUAL_CFG
    , INI_ERR_BAD_FORMAT
    , INI_ERR_NULL_SECTION_NAME
    , INI_ERR_NULL_KEY
    , INI_ERR_REPEATED_SECTION
    , INI_ERR_REPEATED_ITEM
    , INI_ERR_INVALID_SECTION_NAME
    , INI_ERR_INVALID_KEY
    , INI_ERR_SECTION_MISMATCHED
    , INI_ERR_ITEM_MISMATCHED
    , INI_ERR_SECTION_NOT_FOUND
    , INI_ERR_ITEM_NOT_FOUND
};

static const char* const S_ERRORS[] = {
    "Unknown error"
    , "Not implemented"
    , "Failed to allocate memory"
    , "String too long"
    , "Unknown node type"
    , "Not a section node"
    , "Not an item node"
    , "Orphan item"
    , "I/O error"
    , "Comment not allowed"
    , "Residual configuration"
    , "Bad format"
    , "Null section name"
    , "Null key"
    , "Repeated section"
    , "Repeated item"
    , "Invalid section name"
    , "Invalid key"
    , "Section mismatched"
    , "Item mismatched"
    , "Section not found"
    , "Item not found"
};

const char* ini_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    return S_ERRORS[-error_code - 1];
}

static const char *s_newline = "\n";

void ini_set_newline(const char* const newline)
{
    if (NULL != newline &&
        (
            ('\n' == newline[0] && '\0' == newline[1]) ||
            ('\r' == newline[0] && '\n' == newline[1] && '\0' == newline[2])
        )
    )
    {
        s_newline = newline;
    }
}

static char *s_indent_spaces = NULL;

static void free_indent_spaces(void)
{
    if (NULL != s_indent_spaces)
    {
        free(s_indent_spaces);
        s_indent_spaces = NULL;
    }
}

void ini_set_item_indent_width(size_t width)
{
    char *tmp = NULL;

    if (width > INI_INDENT_WIDTH_MAX)
        return;

    if (NULL != (tmp = (char *)realloc(s_indent_spaces, width + 1)))
    {
        s_indent_spaces = tmp;
        memset(s_indent_spaces, ' ', width);
        s_indent_spaces[width] = '\0';
        atexit(free_indent_spaces);
    }
}

char ini_node_type(const ini_node_t *node)
{
    return node->type;
}

#define IS_BLANK(ch)            (' ' == (ch) || '\t' == (ch))
#define IS_SPACE(ch)            (' ' == (ch) || '\f' == (ch) || '\n' == (ch) || '\r' == (ch) || '\t' == (ch) || '\v' == (ch))
#define IS_NEWLINE(ch)          ('\n' == (ch) || '\r' == (ch))
#define IS_COMMENT_TAG(ch)      (';' == (ch) || '#' == (ch))

static char* __get_from_string(char *buf, int buf_len, void *str, int str_len)
{
    char *pos = (char *)str;
    int counter = -1;
    const int MAX_LEN = (buf_len <= str_len) ? (buf_len - 1) : str_len;

    while (++counter < MAX_LEN && (!IS_NEWLINE(*pos)) && '\0' != *pos)
    {
        buf[counter] = *pos;
        ++pos;
    }
    buf[counter] = '\0';

    return (MAX_LEN > 0) ? pos : NULL;
}

static char* __get_from_stream(char *buf, int buf_len, void *stream, int unused_len)
{
    return fgets(buf, buf_len, (FILE *)stream);
}

ini_doc_t* __parse_from(void *target, int target_size, char* (*getline_func)(char*, int, void*, int),
    int strip_blanks, ini_summary_t *nullable_summary)
{
    ini_summary_t summary = { 0 }; /* Use it instead of nullable_summary for reducing the "if" statements. */
    ini_doc_t *doc = (ini_doc_t *)calloc(1, sizeof(ini_doc_t));
    ini_node_t *section = (NULL == doc) ? NULL : doc->section;
    ini_node_t *prev = NULL;
    char *buf = (NULL == doc) ? NULL : (char *)calloc(INI_LINE_SIZE_MAX + 1, sizeof(char));
    int is_str = (target_size > 0);
    char *pos = NULL;

    #define RETURN_IF_TRUE(conditions, errcode, free_mem_statements)   do { \
        if (conditions) { \
            if (NULL != doc) {\
                ini_destroy(doc); \
                doc = NULL; \
            } \
            free_mem_statements; \
            if (NULL != nullable_summary) { \
                memcpy(nullable_summary, &summary, sizeof(ini_summary_t)); \
                nullable_summary->error_code = errcode; \
            } \
            return NULL; \
        } \
    } while (0)

    RETURN_IF_TRUE(NULL == doc, -INI_ERR_MEM_ALLOC, ;);

    while (NULL != (pos = getline_func(buf, INI_LINE_SIZE_MAX + 1, target, target_size)))
    {
        size_t length = 0;
        char *head = buf;
        char *tail = NULL;
        int is_blank_line = 0;
        int is_comment = 0;
        int is_section = 0;
        ini_node_t *_this = (ini_node_t *)calloc(1, sizeof(ini_node_t));

        RETURN_IF_TRUE(NULL == _this, -INI_ERR_MEM_ALLOC, free(buf));

        if (is_str)
        {
            target_size -= (pos - ((char *)target) + 1);
            target = pos + 1;
        }

        while (IS_SPACE(*head)) ++head;

        length = strlen(head);
        is_blank_line = (0 == length);
        is_comment = IS_COMMENT_TAG(head[0]);
        is_section = ('[' == head[0]);

        tail = head + length - 1;
        if (!is_blank_line)
        {
            while (IS_NEWLINE(*tail)) --tail;

            if (is_section || is_comment || strip_blanks)
                while (IS_BLANK(*tail)) --tail;

            length = tail - head + 1;
        }

        if (is_blank_line)
        {
            _this->type = INI_NODE_BLANK_LINE;

            if (NULL == section && NULL == doc->preamble)
                doc->preamble = _this;

            if (NULL != section && NULL == ((ini_section_t *)section->detail)->sub)
                ((ini_section_t *)section->detail)->sub = _this;

            summary.blank_lines += 1;
        }
        else if (is_comment)
        {
            _this->type = INI_NODE_COMMENT;

            _this->detail = calloc(length + 1, sizeof(char));
            RETURN_IF_TRUE(NULL == _this->detail, -INI_ERR_MEM_ALLOC, free(_this);free(buf));
            strncpy((char *)_this->detail, head, length);

            if (NULL == section && NULL == doc->preamble)
                doc->preamble = _this;

            if (NULL != section && NULL == ((ini_section_t *)section->detail)->sub)
                ((ini_section_t *)section->detail)->sub = _this;

            summary.comment_lines += 1;
        }
        else if (is_section)
        {
            char *name = NULL;
            size_t counter = 0;
            int has_extra_brackets = 0;

            RETURN_IF_TRUE(1 == length || ']' == head[1], -INI_ERR_NULL_SECTION_NAME, free(_this);free(buf));
            RETURN_IF_TRUE(']' != head[length - 1], -INI_ERR_BAD_FORMAT, free(_this);free(buf));

            ++head;
            --tail;
            while (IS_BLANK(*head)) ++head;
            RETURN_IF_TRUE(head > tail, -INI_ERR_NULL_SECTION_NAME, free(_this);free(buf));
            while (IS_BLANK(*tail)) --tail;
            RETURN_IF_TRUE(head > tail, -INI_ERR_NULL_SECTION_NAME, free(_this);free(buf));
            length = tail - head + 1;
            while (counter < length && '[' != head[counter] && ']' != head[counter]) ++counter;
            has_extra_brackets = (counter < length);
            RETURN_IF_TRUE(has_extra_brackets, -INI_ERR_BAD_FORMAT, free(_this);free(buf));

            _this->type = INI_NODE_SECTION;

            _this->detail = calloc(1, sizeof(ini_section_t));
            RETURN_IF_TRUE(NULL == _this->detail, -INI_ERR_MEM_ALLOC, free(_this);free(buf));

            name = (char *)calloc(length + 1, sizeof(char));
            RETURN_IF_TRUE(NULL == name, -INI_ERR_MEM_ALLOC, free(_this->detail);free(_this);free(buf));
            strncpy(name, head, length);
            ((ini_section_t *)_this->detail)->name = name;

            if (NULL == section)
                doc->section = _this;
            else
                section->next = _this;

            section = _this;

            prev = NULL; /* NOTE: An non-section node cannot be its precedent. */

            summary.section_lines += 1;
        }
        else
        {
            char *key = NULL;
            char *val = NULL;
            int is_orphan = (NULL == section);
            char *equal_sign = is_orphan ? NULL : strchr(head, '=');
            char *val_head = (NULL == equal_sign) ? NULL : equal_sign + 1;

            RETURN_IF_TRUE(is_orphan || NULL == equal_sign || equal_sign == head,
                is_orphan ? -INI_ERR_ORPHAN_ITEM : ((NULL == equal_sign) ? -INI_ERR_BAD_FORMAT : -INI_ERR_NULL_KEY),
                free(_this);free(buf));

            _this->type = INI_NODE_ITEM;

            _this->detail = calloc(1, sizeof(ini_item_t));
            RETURN_IF_TRUE(NULL == _this->detail, -INI_ERR_MEM_ALLOC, free(_this);free(buf));

            if (strip_blanks && NULL != val_head)
                while (IS_BLANK(*val_head)) ++val_head;
            length = (tail >= val_head ) ? (tail - val_head + 1) : 0;
            val = (char *)calloc(length + 1, sizeof(char));
            RETURN_IF_TRUE(NULL == val, -INI_ERR_MEM_ALLOC, free(_this->detail);free(_this);free(buf));
            strncpy(val, val_head, length);
            ((ini_item_t *)_this->detail)->val = val;

            tail = equal_sign - 1;
            while (IS_BLANK(*tail)) --tail;
            length = tail - head + 1;
            key = (char *)calloc(length + 1, sizeof(char));
            RETURN_IF_TRUE(NULL == key, -INI_ERR_MEM_ALLOC, free(val);free(_this->detail);free(_this);free(buf));
            strncpy(key, head, length);
            ((ini_item_t *)_this->detail)->key = key;

            if (NULL == ((ini_section_t *)section->detail)->sub)
                ((ini_section_t *)section->detail)->sub = _this;
        }

        if (_this != section)
        {
            if (NULL != prev)
                prev->next = _this;

            prev = _this;
        }

        summary.success_lines += 1;
    } /* while (NULL != fgets() */

    free(buf);

    if (NULL != nullable_summary)
        memcpy(nullable_summary, &summary, sizeof(ini_summary_t));

    return doc;
}

ini_doc_t* ini_parse_from_stream(FILE *stream, int strip_blanks/* = 0 or 1, for item value only */,
    ini_summary_t *nullable_summary/* = NULL if failure reason not cared*/)
{
    return __parse_from(stream, 0, __get_from_stream, strip_blanks, nullable_summary);
}

ini_doc_t* ini_parse_from_buffer(char *buf, size_t buf_len, int strip_blanks/* = 0 or 1, for item value only */,
    ini_summary_t *nullable_summary/* = NULL if failure reason not cared*/)
{
    return __parse_from(buf, buf_len, __get_from_string, strip_blanks, nullable_summary);
}

/*
 * ================
 *  TRAVERSAL
 * ================
 */

static ini_summary_t __traverse_nodes_of(ini_node_t *sec, int should_free_memory,
    ini_traverval_callback_t cb, void *cb_arg)
{
    ini_summary_t summary = { 0 };
    int is_section = (NULL != sec && INI_NODE_SECTION == sec->type);
    ini_node_t *node = is_section ? ((ini_section_t *)sec->detail)->sub : NULL;
    const char *sec_name = is_section ? ((ini_section_t *)sec->detail)->name : "";

    while (NULL != node)
    {
        char node_type = node->type;
        ini_node_t *node_ptr = node;

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
                int *counter = (INI_NODE_ITEM == node_type) ? NULL
                    : ((INI_NODE_COMMENT == node_type) ? &summary.comment_lines : &summary.blank_lines);

                summary.success_lines += 1;
                if (NULL != counter)
                    *counter += 1;
            }
        }

        if (should_free_memory)
        {
            if (INI_NODE_ITEM == node_type)
            {
                free(((ini_item_t *)node_ptr->detail)->key);
                free(((ini_item_t *)node_ptr->detail)->val);
            }

            if (INI_NODE_BLANK_LINE != node_type)
                free(node_ptr->detail);

            free(node_ptr);
        }
    } /* while (NULL != node) */

    return summary;
}

static ini_summary_t __traverse_all_nodes(ini_doc_t *doc, int should_free_memory,
    ini_traverval_callback_t cb, void *cb_arg)
{
    ini_summary_t summary = { 0 };
    /* NOTE: error: initializer element is not computable at load time [-Werror=pedantic] */
    ini_node_t *headers[] = { NULL, NULL }/* { doc->preamble, doc->section } */;
    size_t i = 0;

    headers[0] = doc->preamble;
    headers[1] = doc->section;

    for (; i < sizeof(headers) / sizeof(ini_node_t *); ++i)
    {
        ini_node_t *node = headers[i];

        while (NULL != node)
        {
            char node_type = node->type;
            ini_node_t *node_ptr = node;
            const char *sec_name = (INI_NODE_SECTION == node_type) ? ((ini_section_t *)node->detail)->name : "";

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
                    int *counter = (INI_NODE_SECTION == node_type) ? &summary.section_lines
                        : ((INI_NODE_COMMENT == node_type) ? &summary.comment_lines : &summary.blank_lines);

                    summary.success_lines += 1;
                    *counter += 1;
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
                    free(((ini_section_t *)node_ptr->detail)->name);

                if (INI_NODE_BLANK_LINE != node_type)
                    free(node_ptr->detail);

                free(node_ptr);
            }
        } /* while (NULL != node) */
    }

    return summary;
}

static int __dump_node_to_stream(const char *sec_name, ini_node_t *cur_node, void *file_stream)
{
    FILE *stream = (FILE *)file_stream;
    int bytes_written = -INI_ERR_IO;

    switch (cur_node->type)
    {
    case INI_NODE_BLANK_LINE:
        bytes_written = fputs(s_newline, stream);
        break;

    case INI_NODE_COMMENT:
        bytes_written = fputs((const char *)cur_node->detail, stream);
        break;

    case INI_NODE_SECTION:
        bytes_written = fputc('[', stream);
        if (bytes_written > 0)
            bytes_written += fputs(((ini_section_t *)cur_node->detail)->name, stream);
        if (bytes_written > 0)
            bytes_written += fputc(']', stream);
        break;

    case INI_NODE_ITEM:
        if (NULL == s_indent_spaces)
            bytes_written = 0;
        else
            bytes_written = fputs(s_indent_spaces, stream);
        if (bytes_written >= 0)
            bytes_written += fputs(((ini_item_t *)cur_node->detail)->key, stream);
        if (bytes_written > 0)
            bytes_written += fputs("=", stream);
        if (bytes_written > 0)
            bytes_written += fputs(((ini_item_t *)cur_node->detail)->val, stream);
        break;

    default:
        return -INI_ERR_UNKNOWN_NODE_TYPE;
    }

    if (INI_NODE_BLANK_LINE != cur_node->type && bytes_written > 0)
        bytes_written += fputs(s_newline, stream);

    return bytes_written;
}

ini_summary_t ini_dump_to_stream(const ini_doc_t *doc, FILE *stream)
{
    return __traverse_all_nodes((ini_doc_t *)doc, 0, __dump_node_to_stream, stream);
}

typedef struct str_buf_t
{
    char **pptr;
    size_t total;
    size_t used;
    int allow_resizing;
} str_buf_t;

static int __dump_node_to_buffer(const char *sec_name, ini_node_t *cur_node, void *str_buf)
{
    str_buf_t *buf = (str_buf_t *)str_buf;
    void *detail = cur_node->detail;
    char *pos = *(buf->pptr) + buf->used;
    char node_type = cur_node->type;
    size_t newline_len = strlen(s_newline); /* TODO: Add a static variable outside to store this length value. */
    size_t indent_width = strlen(s_indent_spaces); /* TODO: Same as above. */
    size_t name_len = (INI_NODE_SECTION == node_type) ? strlen(((ini_section_t *)detail)->name) : 0; /* TODO: detail->name_len */
    size_t key_len = (INI_NODE_ITEM == node_type) ? strlen(((ini_item_t *)detail)->key) : 0; /* TODO: detail->key_len */
    size_t val_len = (INI_NODE_ITEM == node_type) ? strlen(((ini_item_t *)detail)->val) : 0; /* TODO: detail->val_len */
    size_t comment_len = (INI_NODE_COMMENT == node_type) ? strlen((char *)detail) : 0;
    size_t expected_len = (INI_NODE_ITEM == node_type)
        ? (indent_width + key_len + 1 + val_len + newline_len)
        : ((INI_NODE_SECTION == node_type)
            ? (name_len + 2 + newline_len)
            : ((INI_NODE_COMMENT == node_type) ? (comment_len + newline_len) : newline_len));
    int buf_not_enough = (buf->used + expected_len > buf->total);

    if (NULL == *(buf->pptr) || buf_not_enough)
    {
        char *tmp = (buf_not_enough && !buf->allow_resizing) ? NULL : (char *)realloc(*buf->pptr, buf->total * 2);

        if (NULL == tmp)
            return -INI_ERR_MEM_ALLOC;

        *(buf->pptr) = tmp;
        buf->total *= 2;
        pos = *(buf->pptr) + buf->used;
    }

    switch (node_type)
    {
    case INI_NODE_BLANK_LINE:
        memcpy(pos, s_newline, newline_len);
        break;

    case INI_NODE_COMMENT:
        memcpy(pos, detail, comment_len);
        pos += comment_len;
        memcpy(pos, s_newline, newline_len);
        break;

    case INI_NODE_SECTION:
        *pos = '[';
        ++pos;
        memcpy(pos, ((ini_section_t *)detail)->name, name_len);
        pos += name_len;
        *pos = ']';
        ++pos;
        memcpy(pos, s_newline, newline_len);
        break;

    case INI_NODE_ITEM:
        memcpy(pos, s_indent_spaces, indent_width);
        pos += indent_width;
        memcpy(pos, ((ini_item_t *)detail)->key, key_len);
        pos += key_len;
        *pos = '=';
        ++pos;
        memcpy(pos, ((ini_item_t *)detail)->val, val_len);
        pos += val_len;
        memcpy(pos, s_newline, newline_len);
        break;

    default:
        return -INI_ERR_UNKNOWN_NODE_TYPE;
    }

    buf->used += expected_len;

    return expected_len;
}

ini_summary_t ini_dump_to_buffer(const ini_doc_t *doc, char **buf, size_t *buf_len, int allow_resizing)
{
    str_buf_t arg = { 0 };
    ini_summary_t summary;

    arg.pptr = buf;
    arg.total = (NULL == *buf || 0 == *buf_len) ? 64 : *buf_len;
    arg.allow_resizing = allow_resizing;

    summary = __traverse_all_nodes((ini_doc_t *)doc, 0, __dump_node_to_buffer, &arg);
    if (summary.error_code >= 0)
        *buf_len = arg.total;

    return summary;
}

static int __do_nothing_but_trigger_summary(const char *sec_name, ini_node_t *cur_node, void *none)
{
    return 0;
}

void ini_destroy(ini_doc_t *doc)
{
    if (NULL != doc && __traverse_all_nodes(doc, 1, __do_nothing_but_trigger_summary, NULL).success_lines > 0)
    {
        doc->preamble = NULL;
        doc->section = NULL;
        free(doc);
    }
}

ini_summary_t ini_traverse_all_nodes(ini_doc_t *doc, ini_traverval_callback_t cb, void *cb_arg)
{
    return __traverse_all_nodes(doc, 0, cb, cb_arg);
}

ini_summary_t ini_traverse_all_sections(ini_doc_t *doc, ini_traverval_callback_t cb, void *cb_arg)
{
    ini_summary_t summary = { 0 };
    ini_node_t *node = doc->section;

    if (NULL == cb)
        return summary;

    for (; NULL != node; node = node->next)
    {
        int err = cb(((ini_section_t *)node->detail)->name, node, cb_arg);

        if (err < 0)
        {
            summary.error_code = err;

            return summary;
        }

        summary.success_lines += 1;
        summary.section_lines += 1;
    }

    return summary;
}

ini_summary_t ini_traverse_nodes_of(ini_node_t *sec, ini_traverval_callback_t cb, void *cb_arg)
{
    return __traverse_nodes_of(sec, 0, cb, cb_arg);
}

/*
 * ================
 *  SECTION
 * ================
 */

ini_node_t* ini_section_find(const ini_doc_t *doc, const char *name, size_t name_len/* = 0 if auto calculated later */)
{
    ini_node_t *node = doc->section;

    for (; NULL != node; node = node->next)
    {
        if (INI_NODE_SECTION == node->type && /* TODO: Use name_len */
            0 == strncmp(((ini_section_t *)node->detail)->name, name, INI_LINE_SIZE_MAX - 2))
        {
            return node;
        }
    }

    return NULL;
}

bool ini_section_is_repeated(const ini_doc_t *doc, const char *name, size_t name_len/* = 0 if auto calculated later */)
{
    ini_node_t *first = NULL;
    ini_node_t *node = doc->section;

    for (; NULL != node; node = node->next)
    {
        if (INI_NODE_SECTION == node->type && /* TODO: Use name_len */
            0 == strncmp(((ini_section_t *)node->detail)->name, name, INI_LINE_SIZE_MAX - 2))
        {
            if (NULL != first)
                return true;

            first = node;
        }
    }

    return false;
}

const char* ini_section_get_name(const ini_node_t *sec)
{
    return (INI_NODE_SECTION == sec->type) ? (((ini_section_t *)sec->detail)->name) : NULL;
}

int ini_section_rename(const char *name, size_t name_len, ini_node_t *sec)
{
    ini_section_t *detail = (INI_NODE_SECTION == sec->type) ? ((ini_section_t *)sec->detail) : NULL;
    int is_too_long = (name_len > INI_LINE_SIZE_MAX - 2);
    char *new_name = NULL;
    const char *head = name;
    const char *tail = name + name_len - 1;
    const char *ptr = NULL;

    if (NULL == detail)
        return -INI_ERR_NOT_SECTION_NODE;

    if (is_too_long)
        return -INI_ERR_STRING_TOO_LONG;

    if (0 == name_len)
        return 0;

    while (IS_SPACE(*head)) ++head;
    while (IS_SPACE(*tail)) --tail;
    if (head > tail)
        return 0;

    for (ptr = head; ptr <= tail; ++ptr)
    {
        if (IS_NEWLINE(*ptr) || '\f' == *ptr || '\v' == *ptr || '[' == *ptr || ']' == *ptr)
            return -INI_ERR_BAD_FORMAT;
    }

    name_len = tail - head + 1;

    if (NULL == (new_name = (char *)malloc(name_len + 1)))
        return -INI_ERR_MEM_ALLOC;

    strncpy(new_name, name, name_len);
    new_name[name_len] = '\0';

    free(detail->name);
    detail->name = new_name;

    return name_len;
}

int ini_section_add(const char *name, size_t name_len, ini_doc_t *doc)
{
    return -INI_ERR_NOT_IMPLEMENTED; /* TODO */
}

int ini_section_remove(const char *name, size_t name_len/* = 0 if auto calculated later */, ini_doc_t *doc)
{
    return -INI_ERR_NOT_IMPLEMENTED; /* TODO */
}

/*
 * ================
 *  ITEM
 * ================
 */

ini_node_t* ini_item_find(const ini_node_t *sec, const char *key, size_t key_len/* = 0 if auto calculated later */)
{
    ini_node_t *node = (INI_NODE_SECTION == sec->type) ? (((ini_section_t *)sec->detail)->sub) : NULL;

    for (; NULL != node; node = node->next)
    {
        if (INI_NODE_ITEM == node->type && /* TODO: Use key_len */
            0 == strncmp(((ini_item_t *)node->detail)->key, key, INI_KEY_SIZE_MAX))
        {
            return node;
        }
    }

    return NULL;
}

bool ini_item_is_repeated(const ini_node_t *sec, const char *key, size_t key_len/* = 0 if auto calculated later */)
{
    ini_node_t *first = NULL;
    ini_node_t *node = (INI_NODE_SECTION == sec->type) ? (((ini_section_t *)sec->detail)->sub) : NULL;

    for (; NULL != node; node = node->next)
    {
        if (INI_NODE_ITEM == node->type && /* TODO: Use key_len */
            0 == strncmp(((ini_item_t *)node->detail)->key, key, INI_KEY_SIZE_MAX))
        {
            if (NULL != first)
                return true;

            first = node;
        }
    }

    return false;
}

const char* ini_item_get_key(const ini_node_t *item)
{
    return (INI_NODE_ITEM == item->type) ? (((ini_item_t *)item->detail)->key) : NULL;
}

int ini_item_set_key(const char *key, size_t key_len, ini_node_t *item)
{
    ini_item_t *detail = (INI_NODE_ITEM == item->type) ? ((ini_item_t *)item->detail) : NULL;
    int is_too_long = (key_len > INI_KEY_SIZE_MAX);
    char *new_key = NULL;
    const char *head = key;
    const char *tail = key + key_len - 1;
    const char *ptr = NULL;

    if (NULL == detail)
        return -INI_ERR_NOT_ITEM_NODE;

    if (is_too_long)
        return -INI_ERR_STRING_TOO_LONG;

    if (0 == key_len)
        return 0;

    while (IS_SPACE(*head)) ++head;
    while (IS_SPACE(*tail)) --tail;
    if (head > tail)
        return 0;

    if (IS_COMMENT_TAG(head[0]) || '[' == head[0])
        return -INI_ERR_BAD_FORMAT;

    for (ptr = head; ptr <= tail; ++ptr)
    {
        if (IS_NEWLINE(*ptr) || '\f' == *ptr || '\v' == *ptr)
            return -INI_ERR_BAD_FORMAT;
    }

    key_len = tail - head + 1;

    if (NULL == (new_key = (char *)malloc(key_len + 1)))
        return -INI_ERR_MEM_ALLOC;

    strncpy(new_key, head, key_len);
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
    int is_too_long = (val_len > INI_LINE_SIZE_MAX - INI_KEY_SIZE_MAX - 1);
    char *new_value = NULL;
    const char *head = val;
    const char *tail = val + val_len - 1;
    const char *ptr = NULL;

    if (NULL == detail)
        return -INI_ERR_NOT_ITEM_NODE;

    if (is_too_long)
        return -INI_ERR_STRING_TOO_LONG;

    if (head > tail)
        return 0;

    for (ptr = head; ptr <= tail; ++ptr)
    {
        if (IS_NEWLINE(*ptr) || '\f' == *ptr || '\v' == *ptr)
            return -INI_ERR_BAD_FORMAT;
    }

    if (NULL == (new_value = (char *)malloc(val_len + 1)))
        return -INI_ERR_MEM_ALLOC;

    strncpy(new_value, head, val_len);
    new_value[val_len] = '\0';

    free(detail->val);
    detail->val = new_value;

    return val_len;
}

int ini_item_add(const char *key, size_t key_len, const char *val, size_t val_len, ini_node_t *sec)
{
    return -INI_ERR_NOT_IMPLEMENTED; /* TODO */
}

int ini_item_remove(const char *key, size_t key_len/* = 0 if auto calculated later */, ini_node_t *sec)
{
    return -INI_ERR_NOT_IMPLEMENTED; /* TODO */
}

/*
 * ================
 *  COMMENT
 * ================
 */

const char* ini_comment_get(const ini_node_t *node)
{
    return (INI_NODE_COMMENT == node->type) ? ((char *)node->detail) : NULL;
}

int ini_comment_set(const char *comment, size_t comment_len, ini_node_t *node)
{
    int is_too_long = (comment_len > INI_LINE_SIZE_MAX);
    int allowed = (INI_NODE_COMMENT == node->type || INI_NODE_BLANK_LINE == node->type);
    char *new_comment = NULL;
    const char *head = comment;
    const char *tail = comment + comment_len - 1;
    const char *ptr = NULL;

    if (is_too_long)
        return -INI_ERR_STRING_TOO_LONG;

    if (!allowed)
        return -INI_ERR_COMMENT_NOT_ALLOWED;

    if (0 == comment_len)
        return 0;

    while (IS_SPACE(*head)) ++head;
    while (IS_SPACE(*tail)) --tail;
    if (head > tail)
        return 0;

    if (!IS_COMMENT_TAG(head[0]))
        return -INI_ERR_BAD_FORMAT;

    for (ptr = head; ptr <= tail; ++ptr)
    {
        if (IS_NEWLINE(*ptr) || '\f' == *ptr || '\v' == *ptr)
            return -INI_ERR_BAD_FORMAT;
    }

    comment_len = tail - head + 1;

    if (NULL == (new_comment = (char *)malloc(comment_len + 1)))
        return -INI_ERR_MEM_ALLOC;

    strncpy(new_comment, head, comment_len);
    new_comment[comment_len] = '\0';

    if (NULL != node->detail)
        free(node->detail);
    node->detail = new_comment;
    if (INI_NODE_BLANK_LINE == node->type)
        node->type = INI_NODE_COMMENT;

    return comment_len;
}

#ifdef TEST

#include <ctype.h>
#include <errno.h>

static void print_summary(const ini_summary_t *summary, const char *title, FILE *stream)
{
    fprintf(stream, "%s: successes: %d, sections: %d, items: %d, comment lines: %d, blank lines: %d\n",
        title, summary->success_lines, summary->section_lines,
        summary->success_lines - summary->section_lines - summary->comment_lines - summary->blank_lines,
        summary->comment_lines, summary->blank_lines);
}

static int test_getters_and_setters(const char *sec_name, ini_node_t *cur_node, void *doc)
{
    const char* const STR_TO_APPEND = "\t TEST \t";
    const size_t APPEND_LEN = strlen(STR_TO_APPEND);
    char buf[INI_LINE_SIZE_MAX + 1] = { 0 };
    ini_node_t *sec = NULL;
    const char *key = NULL;

    switch (ini_node_type(cur_node))
    {
    case INI_NODE_BLANK_LINE:
        break;

    case INI_NODE_COMMENT:
        strcpy(buf, ini_comment_get(cur_node));
        strncat(buf, STR_TO_APPEND, APPEND_LEN);

        return ini_comment_set(buf, strlen(buf), cur_node);

    case INI_NODE_SECTION:
        if (ini_section_get_name(cur_node) != sec_name)
            return -INI_ERR_SECTION_MISMATCHED;

        if (ini_section_is_repeated((ini_doc_t *)doc, sec_name, strlen(sec_name)))
            return -INI_ERR_REPEATED_SECTION;

        strcpy(buf, sec_name);
        strncat(buf, STR_TO_APPEND, APPEND_LEN);

        return ini_section_rename(buf, strlen(buf), cur_node);

    case INI_NODE_ITEM:
        sec = ini_section_find((ini_doc_t *)doc, sec_name, strlen(sec_name));
        key = ini_item_get_key(cur_node);

        if (ini_item_is_repeated(sec, key, 0) || ini_item_find(sec, key, 0) != cur_node)
            return -INI_ERR_REPEATED_ITEM;

        strcpy(buf, ini_item_get_value(cur_node));
        strncat(buf, STR_TO_APPEND, APPEND_LEN);

        ini_item_set_value(buf, strlen(buf), cur_node);

        strcpy(buf, key);
        strncat(buf, STR_TO_APPEND, APPEND_LEN);

        return ini_item_set_key(buf, strlen(buf), cur_node);

    default:
        return -INI_ERR_UNKNOWN_NODE_TYPE;
    }

    return 0;
}

int main(int argc, char **argv)
{
    char buf[INI_LINE_SIZE_MAX + 1] = { 0 };
    const char* const NEWLINE_TYPES[] = { "\n", "\r\n" };
    const char* const DEFAULT_RESULT_PATH = "ini_file_test.ini";
    char path[INI_LINE_SIZE_MAX + 1] = { 0 };
    char *begin = NULL, *end = NULL;
    FILE *wstream = NULL, *rstream = NULL;
    ini_doc_t *doc = NULL;
    char *ini_str_buf = NULL;
    size_t ini_str_len = 0;
    ini_summary_t summary = { 0 };

    printf("Step 1: Please specify the indent width (from 0 to 16) [0]: ");
    ini_set_item_indent_width(atoi(fgets(buf, sizeof(buf), stdin)) % (INI_INDENT_WIDTH_MAX + 1));

    printf("Step 2: Please choose the newline charactor: 1. \\n; 2. \\r\\n\n");
    printf("        Your choice [1]: ");
    ini_set_newline(NEWLINE_TYPES[atoi(fgets(buf, sizeof(buf), stdin)) - 1]);

    printf("Step 3: Please specify the result path [%s]: ", DEFAULT_RESULT_PATH);
    begin = fgets(buf, sizeof(buf), stdin);
    end = begin + strlen(buf) - 1;
    while (isspace(*begin)) ++begin;
    while (isspace(*end)) --end;
    if (end >= begin)
        strncpy(path, begin, end - begin + 1);
    else
        strncpy(path, DEFAULT_RESULT_PATH, sizeof(path));

    printf("Step 4: Please enter contents of your .ini file, and press Ctrl+D to start test:\n");
    doc = ini_parse_from_stream(stdin, /*strip_blanks = */0, &summary);
    print_summary(&summary, "Parse[1] summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to parse the input, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
        goto TEST_END;
    }

    summary = ini_dump_to_stream(doc, stdout);
    print_summary(&summary, "Dump[1] summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to dump the ini memory, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
        goto TEST_END;
    }

    summary = ini_dump_to_buffer(doc, &ini_str_buf, &ini_str_len, /*allow_resizing = */1);
    print_summary(&summary, "Dump[2] summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to dump the ini memory, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
        goto TEST_END;
    }
    printf("Capacity of ini_str_buf has expanded from 0 to %d, contents:\n%s\n", (int)ini_str_len, ini_str_buf);

    ini_destroy(doc);
    doc = ini_parse_from_buffer(ini_str_buf, strlen(ini_str_buf), /*strip_blanks =*/1, &summary);
    print_summary(&summary, "Parse[2] summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to parse the input, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
        goto TEST_END;
    }

    summary = ini_traverse_all_nodes(doc, test_getters_and_setters, doc);
    print_summary(&summary, "Getters-and-Setters test summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to test getters and setters, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
        goto TEST_END;
    }

    wstream = fopen(path, "w");
    if (NULL == wstream)
    {
        fprintf(stderr, "%s: failed to open this file, reason: %s\n", path, strerror(errno));
        summary.error_code = -INI_ERR_IO;
        goto TEST_END;
    }

    summary = ini_dump_to_stream(doc, wstream);
    print_summary(&summary, "Dump[3] summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to dump the ini memory, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
        goto TEST_END;
    }
    if (0 != fflush(wstream))
    {
        fprintf(stderr, "%s: failed to flush memory to this file, reason: %s\n", path, strerror(errno));
        summary.error_code = -INI_ERR_IO;
        goto TEST_END;
    }

    rstream = fopen(path, "r");
    if (NULL == rstream)
    {
        fprintf(stderr, "%s: failed to open this file, reason: %s\n", path, strerror(errno));
        summary.error_code = -INI_ERR_IO;
        goto TEST_END;
    }

    ini_destroy(doc);
    doc = ini_parse_from_stream(rstream, /*strip_blanks =*/1, &summary);
    print_summary(&summary, "Parse[3] summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to parse the input, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
        goto TEST_END;
    }

    summary = ini_dump_to_stream(doc, stdout);
    print_summary(&summary, "Dump[3] summary", (summary.error_code < 0) ? stderr : stdout);
    if (summary.error_code < 0)
    {
        fprintf(stderr, "Failed to dump the ini memory, error occured at line %d: %s\n",
            summary.success_lines + 1, ini_error(summary.error_code));
    }

TEST_END:

    if (NULL != ini_str_buf)
        free(ini_str_buf);

    ini_destroy(doc);

    if (NULL != wstream)
        fclose(wstream);

    if (NULL != rstream)
        fclose(rstream);

    return (summary.error_code < 0) ? -1 : 0;
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
 *
 * >>> 2021-12-20, Man Hung-Coeng:
 *  01. Fix errors in the existing functions.
 *  02. Implement ini_parse() and rename it to ini_parse_from_stream();
 *      rename ini_dump() to ini_dump_to_stream().
 *  03. Add ini_{section,item}_is_repeated(), ini_traverse_all_sections(),
 *      ini_dump_to_buffer() (not implemented)
 *      and ini_parse_from_buffer() (not implemented).
 *  04. Finish test function.
 *
 * >>> 2021-12-21, Man Hung-Coeng:
 *  01. Remove unnecessary parameters from ini_{section,item}_remove().
 *  02. Change return type of ini_parse_from_{stream,buffer}() to ini_cfg_t*
 *      in order to solve the "incomplete type" error of ini_cfg_t
 *      when the two functions are called outside this file.
 *  03. Change parameter type of ini_destroy() from ini_cfg_t* to ini_cfg_t**.
 *
 * >>> 2021-12-22, Man Hung-Coeng:
 *  01. Fix errors in INI_NODE_ITEM case of __dump_node().
 *  02. Implement ini_parse_from_buffer() and ini_dump_to_buffer().
 *  03. Re-add *_len parameter to some functions for future optimization.
 *
 * >>> 2021-12-31, Man Hung-Coeng:
 *  01. Rename file simple_ini_config.{c,h} to ini_file.{c,h}.
 *  02. Rename structure ini_cfg_t to ini_doc_t.
 *  03. Change return type of ini_{section,item}_is_repeated() to bool.
 *
 * >>> 2022-01-08, Man Hung-Coeng:
 *  01. Change parameter type of ini_destroy() back to ini_cfg_t*.
 *  02. Use atexit() to register the newly added function free_indent_spaces()
 *      within ini_set_item_indent_width() in order to free the dynamically
 *      allocated memory on exit.
 *  03. Use the allow_resizing parameter within the str_buf argument to decide
 *      whether memory re-allocation is allowed within __dump_node_to_buffer().
 *  04. Improve code style according to suggestions from cppcheck and clang.
 *
 * >>> 2022-05-16, Man Hung-Coeng:
 *  01. Eliminate warnings from VIM plugins.
 */

