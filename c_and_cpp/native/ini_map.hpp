/*
 * A read-only class based on std::map and the relative C APIs
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
#include <exception>

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

    class cstr_map : public std::map<const char*, const char*, char_lt_op>
    {
    public:
        const char* operator[](const char *key) const
        {
            cstr_map::const_iterator iter = this->find(key);
            const char *safe_key = ((nullptr == key) ? "<nullptr>" : key);

            if (this->end() == iter)
                throw std::invalid_argument(std::string("INI configuration key not found: ") + safe_key);

            return iter->second;
        }
    };

    typedef std::map<const char*, cstr_map, char_lt_op> section_map;

    enum
    {
        ERR_NULL_PARAM = 1
        , INI_ERR_NOT_IMPLEMENTED
        , ERR_MEM_ALLOC
    };

public:
    ini_map() = delete;

    explicit ini_map(ini_doc_t *doc, const char *path = nullptr)
        : err_(-ERR_NULL_PARAM)
        , doc_(doc)
        , path_(nullptr)
        , map_(nullptr)
    {
        construct(doc, path);
    }

    ini_map(const ini_map &src)
        : err_(-ERR_NULL_PARAM)
        , doc_(nullptr)
        , path_(nullptr)
        , map_(nullptr)
    {
        construct(src.doc_, src.path_);
    }

    ini_map(ini_map &&src)
        : err_(src.err_)
        , doc_(src.doc_)
        , path_(src.path_)
        , map_(src.map_)
    {
        src.path_ = nullptr;
        src.map_ = nullptr;
    }

    ini_map& operator=(const ini_map &src)
    {
        if (this != &src)
        {
            ini_doc_t *doc_bak = src.doc_;
            const char *path_bak = src.path_;

            this->~ini_map();

            construct(doc_bak, path_bak);
        }

        return *this;
    }

    ini_map& operator=(ini_map &&src)
    {
        if (this != &src)
        {
            this->err_ = src.err_;
            this->doc_ = src.doc_;
            this->path_ = src.path_;
            src.path_ = nullptr;
            this->map_ = src.map_;
            src.map_ = nullptr;
        }

        return *this;
    }

    ~ini_map()
    {
        if (nullptr != map_)
        {
            delete map_;
            map_ = nullptr;
        }

        if (nullptr != path_)
        {
            delete[] path_;
            path_ = nullptr;
        }
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
        section_map::const_iterator iter = map_->find(section);
        const char *safe_sec = ((nullptr == section) ? "<nullptr>" : section);

        if (map_->end() == iter)
            throw std::invalid_argument(std::string("INI configuration section not found: [") + safe_sec + "]");

        return iter->second;
    }

    inline section_map::const_iterator begin(void) const
    {
        return map_->begin();
    }

    inline section_map::const_iterator end(void) const
    {
        return map_->end();
    }

    inline int sync(void)
    {
        if (err_ < 0)
            return err_;

        map_->clear();

        return (err_ = ini_traverse_all_nodes(doc_, save_node_into_map, map_).error_code);
    }

private:
    void construct(ini_doc_t *doc, const char *path)
    {
        err_ = -ERR_NULL_PARAM;

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

    static int save_node_into_map(const char *section, ini_node_t *cur_node, void *map)
    {
        section_map *sec_map = (section_map *)map;

        switch (ini_node_type(cur_node))
        {
        case INI_NODE_SECTION:
            if (sec_map->end() == sec_map->find(section))
                sec_map->insert(std::make_pair(section, cstr_map()));

            break;

        case INI_NODE_ITEM:
            {
                cstr_map &item_map = sec_map->find(section)->second;
                const char *key = ini_item_get_key(cur_node);

                if (item_map.end() == item_map.find(key))
                    item_map.insert(std::make_pair(key, ini_item_get_value(cur_node)));
            }

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
 *
 * >>> 2022-01-03, Man Hung-Coeng:
 *  01. Re-declare copy/move constructors and copy/move assignment operators
 *      and implement them.
 *  02. Turn the index operator [] into a read-only one
 *      to avoid producing unexpected data.
 *  03. Rename refresh() to sync().
 *
 * >>> 2022-02-10, Man Hung-Coeng:
 *  01. Change the exception type of operator [] from const char*
 *      to std::invalid_argument to make the exception reason displayable
 *      even if the exception is not catched.
 */


