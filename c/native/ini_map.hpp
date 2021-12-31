/*
 * A tiny class based on std::map and the relative C APIs
 * to make it easier to access parsed INI data.
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

#ifndef __INLINE_INI_MAP_HPP__
#define __INLINE_INI_MAP_HPP__

#include <string.h>

#include "ini_file.h"

#include <map>

class ini_map final
{
private:
    typedef struct char_less_than_operator
    {
        bool operator()(const char *s1, const char *s2) const
        {
            return strcmp(s1, s2) < 0;
        }
    } char_less_than_operator;

    typedef char_less_than_operator char_lt_op;

    typedef std::map<const char*, const char*, char_lt_op> cstr_map;

    typedef std::map<const char*, cstr_map, char_lt_op> section_map;

    enum
    {
        ERR_NULL_PARAM = 1
        , ERR_MEM_ALLOC
    };

public:
    ini_map() = delete;
    ini_map(const ini_map&) = delete;
    ini_map(const ini_map&&) = delete;
    ini_map& operator=(const ini_map&) = delete;
    ini_map& operator=(const ini_map&&) = delete;

    ini_map(ini_doc_t *doc, const char *path = nullptr)
        : err_(-ERR_NULL_PARAM)
        , doc_(doc)
        , path_(nullptr)
        , map_(nullptr)
    {
        if (nullptr == doc_)
            return;

        if (nullptr != path)
        {
            if (nullptr == (path_ = new char[strlen(path) + 1]))
            {
                err_ = -ERR_MEM_ALLOC;
                return;
            }

            strcpy(path_, path);
        }

        if (nullptr == (map_ = new section_map()))
        {
            err_ = -ERR_MEM_ALLOC;
            return;
        }

        err_ = ini_traverse_all_nodes(doc_, save_node_into_map, map_).error_code;
    }

    ~ini_map()
    {
        if (nullptr != map_)
            delete map_;

        if (nullptr != path_)
            delete[] path_;
    }

    inline int error_code(void) const
    {
        return err_;
    }

    const char* error_string(void) const
    {
        if (err_ >= 0)
            return "OK";

        switch (err_)
        {
        case -ERR_NULL_PARAM:
            return "Null parameter";

        case -ERR_MEM_ALLOC:
            return "Failed to allocate memory";

        default:
            break;
        }

        return ini_error(err_);
    }

    inline ini_doc_t* document(void)
    {
        return doc_;
    }

    inline const char* path(void) const
    {
        return path_;
    }

    inline const cstr_map& operator[](const char *section) const
    {
        return (*map_)[section];
    }

    inline cstr_map& operator[](const char *section) // TODO: Set to private?
    {
        return (*map_)[section];
    }

    section_map::iterator begin(void)
    {
        return map_->begin();
    }

    section_map::iterator end(void)
    {
        return map_->end();
    }

    int refresh(void)
    {
        if (err_ < 0)
            return err_;

        map_->clear();

        return (err_ = ini_traverse_all_nodes(doc_, save_node_into_map, map_).error_code);
    }

private:
    static int save_node_into_map(const char *sec_name, ini_node_t *cur_node, void *map)
    {
        switch (ini_node_type(cur_node))
        {
        case INI_NODE_ITEM:
            (*((section_map *)map))[sec_name][ini_item_get_key(cur_node)] = ini_item_get_value(cur_node);
            break;

        default:
            break;
        }

        return 0;
    }

private:
    int err_;
    ini_doc_t *doc_;
    char *path_;
    section_map *map_;
};

#endif /* #ifndef __INLINE_INI_MAP_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2021-12-31, Man Hung-Coeng:
 *  01. Create.
 */


