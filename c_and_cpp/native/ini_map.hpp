/*
 * A read-only class based on std::map and the relative C APIs
 * to make it easier to access parsed INI data.
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

#ifndef __INLINE_INI_MAP_HPP__
#define __INLINE_INI_MAP_HPP__

#include <string.h>

#include "ini_file.h"

#include <map>
#include <stdexcept>

class ini_map_c final
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

    class cstr_map_c : public std::map<const char*, const char*, char_lt_op>
    {
    public:
        const char* operator[](const char *key) const
        {
            cstr_map_c::const_iterator iter = this->find(key);
            const char *safe_key = ((nullptr == key) ? "<nullptr>" : key);

            if (this->end() == iter)
                throw std::invalid_argument(std::string("INI configuration key not found: ") + safe_key);

            return iter->second;
        }
    };

    typedef std::map<const char*, cstr_map_c, char_lt_op> section_map_t;

    enum
    {
        ERR_NOT_INITIALIZED = 1
        , ERR_NOT_IMPLEMENTED
        , ERR_MEM_ALLOC
    };

public:
    ini_map_c() = delete;

    explicit ini_map_c(ini_doc_t *doc, const char *path = nullptr)
    {
        reset();
        construct(doc, path);
    }

    ini_map_c(const ini_map_c &src)
    {
        reset();
        construct(src.doc_, src.path_);
    }

    ini_map_c(ini_map_c &&src)
        : err_(src.err_)
        , doc_(src.doc_)
        , path_(src.path_)
        , dir_(src.dir_)
        , basename_(src.basename_)
        , map_(src.map_)
    {
        src.reset();
    }

    ini_map_c& operator=(const ini_map_c &src)
    {
        if (this != &src)
        {
            release();
            reset();
            construct(src.doc_, src.path_);
        }

        return *this;
    }

    ini_map_c& operator=(ini_map_c &&src)
    {
        if (this != &src)
        {
            this->err_ = src.err_;
            this->doc_ = src.doc_;
            this->path_ = src.path_;
            this->dir_ = src.dir_;
            this->basename_ = src.basename_;
            this->map_ = src.map_;

            src.reset();
        }

        return *this;
    }

    ~ini_map_c()
    {
        release();
        reset();
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
        case -ERR_NOT_INITIALIZED:
            return "Not initialized";

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

    inline const char* directory(void) const
    {
        return dir_;
    }

    inline const char* basename(void) const
    {
        return basename_;
    }

    inline const cstr_map_c& operator[](const char *section) const
    {
        section_map_t::const_iterator iter = map_->find(section);
        const char *safe_sec = ((nullptr == section) ? "<nullptr>" : section);

        if (map_->end() == iter)
            throw std::invalid_argument(std::string("INI configuration section not found: [") + safe_sec + "]");

        return iter->second;
    }

    inline section_map_t::const_iterator begin(void) const
    {
        return map_->begin();
    }

    inline section_map_t::const_iterator end(void) const
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
    inline void release(void)
    {
        if (nullptr != map_)
            delete map_;

        char **xx[] = { &basename_, &dir_, &path_ };

        for (size_t i = 0; i < sizeof(xx) / sizeof(char **); ++i)
        {
            if (nullptr != *xx[i])
                delete[] *xx[i];
        }
    }

    inline void reset(void)
    {
        err_ = -ERR_NOT_INITIALIZED;
        doc_ = nullptr;
        path_ = nullptr;
        dir_ = nullptr;
        basename_ = nullptr;
        map_ = nullptr;
    }

    void construct(ini_doc_t *doc, const char *path)
    {
        if (nullptr == (doc_ = doc))
            return;

        if (nullptr != path)
        {
            size_t path_len = strlen(path);
            char *cast_path = const_cast<char *>(path); // for fixing the invalid-conversion compile error
            char *slash_ptr = strrchr(cast_path, '/');
            char *delimiter_ptr = (nullptr == slash_ptr) ? strrchr(cast_path, '\\') : slash_ptr;
            size_t dir_len = (nullptr == delimiter_ptr) ? 1 : (delimiter_ptr - path);
            size_t basename_len = (nullptr == delimiter_ptr) ? path_len : strlen(delimiter_ptr + 1);
            struct xx_t
            {
                const char *src;
                char **dest;
                size_t len;
            } xx[] = {
                { path, &path_, path_len }
                , { ((nullptr == delimiter_ptr) ? "." : path), &dir_, dir_len }
                , { ((nullptr == delimiter_ptr) ? path : (delimiter_ptr + 1)), &basename_, basename_len }
            };

            for (size_t i = 0; i < sizeof(xx) / sizeof(struct xx_t); ++i)
            {
                if (nullptr == (*xx[i].dest = new char[xx[i].len + 1]))
                {
                    err_ = -ERR_MEM_ALLOC;
                    return;
                }

                strncpy(*xx[i].dest, xx[i].src, xx[i].len);
                (*xx[i].dest)[xx[i].len] = '\0';
            }
        }

        if (nullptr == (map_ = new section_map_t()))
        {
            err_ = -ERR_MEM_ALLOC;
            return;
        }

        err_ = ini_traverse_all_nodes(doc_, save_node_into_map, map_).error_code;
    }

    static int save_node_into_map(const char *section, ini_node_t *cur_node, void *map)
    {
        section_map_t *sec_map = (section_map_t *)map;

        switch (ini_node_type(cur_node))
        {
        case INI_NODE_SECTION:
            if (sec_map->end() == sec_map->find(section))
                sec_map->insert(std::make_pair(section, cstr_map_c()));

            break;

        case INI_NODE_ITEM:
            {
                cstr_map_c &item_map = sec_map->find(section)->second;
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
    char *dir_;
    char *basename_;
    section_map_t *map_;
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
 *
 * >>> 2022-03-09, Man Hung-Coeng:
 *  01. Rename ini_map to ini_map_c, cstr_map to cstr_map_c
 *      and section_map to section_map_t.
 *
 * >>> 2022-05-01, Man Hung-Coeng:
 *  01. Add member function directory() and basename().
 *  02. Fix some errors of initialization of data members.
 *
 * >>> 2022-05-08, Man Hung-Coeng:
 *  01. Eliminate stupid cppcheck[operatorEqVarError] warnings
 *      happening to operator= !
 *
 * >>> 2022-09-04, Man Hung-Coeng:
 *  01. Replace the header <exception> with <stdexcept>.
 */

