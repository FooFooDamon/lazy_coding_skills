/*
 * Enhancements of string operation that ANSI C does not provide.
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

#ifndef __STRING_ENHANCEMENTS_H__
#define __STRING_ENHANCEMENTS_H__

#include <stddef.h> /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

const char* str_error(int error_code);

char** str_split(const char *str, size_t str_len, const char *delimiter, size_t delimiter_len,
    size_t *nullable_splits, char **nullable_result_holder, int *nullable_errcode);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __STRING_ENHANCEMENTS_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2021-12-30, Man Hung-Coeng:
 *  01. Create.
 */


