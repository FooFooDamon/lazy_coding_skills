/*
 * Supplements to string operation of ANSI C.
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

#include "string_supplements.h"

#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    STR_ERR_UNKNOWN = 1
    , STR_ERR_NOT_IMPLEMENTED
    , STR_ERR_MEM_ALLOC
    , STR_ERR_ZERO_LENGTH
    , STR_ERR_STRING_TOO_LONG
    , STR_ERR_SPLITTING_SKIPPED

    , STR_ERR_END /* NOTE: All error codes should be defined ahead of this. */
};

static const char* const S_ERRORS[] = {
    "Unknown error"
    , "Not implemented"
    , "Failed to allocate memory"
    , "Zero length"
    , "String too long"
    , "Splitting skipped"
};

const char* str_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    if (error_code <= -STR_ERR_END)
        return strerror(-error_code - STR_ERR_END);

    return S_ERRORS[-error_code - 1];
}

static char** __str_split(const char *str, size_t str_len, const char *delimiter, size_t delimiter_len,
    size_t *inout_splits, char **nullable_result_holder, size_t capacity_per_holder_item, int *errcode)
{
    size_t splits = 0;
    char **pptr = nullable_result_holder;

    if (delimiter_len < str_len)
    {
        const int is_dynamic_alloc = (NULL == nullable_result_holder);
        const size_t MAX_SPLITS = (*inout_splits > 0) ? *inout_splits + 1 : (size_t)-1;
        size_t capacity = 1;
        const char *head = str;
        const char *tail = head;
        const char *STOP = str + str_len;
        char **tmp = NULL;

        *errcode = 0;

        while (++splits <= MAX_SPLITS)
        {
            size_t len = 0;

            if (head > STOP)
                break;

            if (1 == delimiter_len)
                while ((tail < STOP) && (*delimiter != *tail)) ++tail;
            else
            {
                tail = strstr(head, delimiter);
                if (NULL == tail || tail > STOP)
                    tail = STOP;
            }

            if (is_dynamic_alloc && (NULL == pptr || splits + 1 > capacity))
            {
                capacity *= 2;
                tmp = (char **)realloc(pptr, sizeof(char *) * capacity);
                if (NULL == tmp)
                {
                    *errcode = -STR_ERR_MEM_ALLOC;

                    break;
                }
                pptr = tmp;
            }

            len = (splits < MAX_SPLITS) ? (tail - head) : (STOP - head);
            if (is_dynamic_alloc)
            {
                if (NULL == (pptr[splits - 1] = (char *)malloc(len + 1)))
                {
                    *errcode = -STR_ERR_MEM_ALLOC;

                    break;
                }
            }
            else
            {
                if (len >= capacity_per_holder_item)
                    len = capacity_per_holder_item - 1;
            }
            memcpy(pptr[splits - 1], head, len);
            pptr[splits - 1][len] = '\0';

            tail += delimiter_len;
            head = tail;
        } /* while (++splits <= MAX_SPLITS) */

        if (is_dynamic_alloc)
            pptr[splits - 1] = NULL;

        if (is_dynamic_alloc && capacity > splits && (NULL != (tmp = (char**)realloc(pptr, sizeof(char *) * splits))))
            pptr = tmp;
    }
    else
    {
        *errcode = -STR_ERR_SPLITTING_SKIPPED;
    } /* if (delimiter_len < str_len) */

    *inout_splits = (delimiter_len < str_len) ? (splits - 2) : 0;

    return pptr;
}

char** str_split(const char *str, size_t str_len, const char *delimiter, size_t delimiter_len,
    size_t max_splits_if_not_zero, int *nullable_errno_or_splits_output)
{
    size_t splits = max_splits_if_not_zero;
    int err = 0;
    char **pptr = __str_split(str, str_len, delimiter, delimiter_len, &splits, NULL, 0, &err);

    if (NULL != nullable_errno_or_splits_output)
        *nullable_errno_or_splits_output = (err < 0) ? err : (int)splits;

    return pptr;
}

int str_split_destroy(char **pptr)
{
    if (NULL != pptr)
    {
        char **tmp = pptr;
        int i = 1;

        for (; NULL != *tmp; ++tmp, ++i)
        {
            free(*tmp);
        }

        free(pptr);

        return i;
    }

    return 0;
}

int str_split_to_fixed_buffer(const char *str, size_t str_len, const char *delimiter, size_t delimiter_len,
    char **buf, size_t buf_items, size_t capacity_per_item)
{
    size_t splits = buf_items - 1;
    int err = (0 == buf_items || 0 == capacity_per_item) ? -STR_ERR_ZERO_LENGTH : 0;

    if (0 == err)
        __str_split(str, str_len, delimiter, delimiter_len, &splits, buf, capacity_per_item, &err);

    return (err < 0) ? err : (int)splits;
}

#ifdef TEST

#include <stdio.h>

#define BUF_ITEM_COUNT      8
#define BUF_ITEM_SIZE       4

int main(int argc, char **argv)
{
    const char *simple_str = "/aa//bbbbb/cccc//";
    size_t simple_len = strlen(simple_str);
    const char *complex_str = "*|*a*|**|*bbb*|*ccccc*|ddd*|*e*|*fff*|*ggggg*|*";
    size_t complex_len = strlen(complex_str);
    int splits = 0;
    char **result_pptr = NULL;
    char **pptr = NULL;
    char buf[BUF_ITEM_COUNT][BUF_ITEM_SIZE] = { 0 };
    char *buf_ptr[BUF_ITEM_COUNT]; /* TODO: Any easier initialization ?? */
    size_t i = 0;

    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i)
    {
        buf_ptr[i] = buf[i];
    }

    result_pptr = str_split(simple_str, simple_len, "/", 1, 0, NULL);
    printf(">>> Case 1: dynamic allocated result; ignored error code.\n");
    printf("str_split(%s, /):\n", simple_str);
    if (NULL != result_pptr)
    {
        for (pptr = result_pptr, i = 0; NULL != *pptr; ++pptr, ++i)
        {
            printf("    [%d]: %s\n", (int)i, *pptr);
        }
        str_split_destroy(result_pptr);
    }

    str_split_to_fixed_buffer(simple_str, simple_len, "/", 1, buf_ptr, BUF_ITEM_COUNT, BUF_ITEM_SIZE);
    printf(">>> Case 2: fixed buffer on stack; ignored error code.\n");
    printf("str_split_to_fixed_buffer(%s, /):\n", simple_str);
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i)
    {
        printf("    [%d]: %s\n", (int)i, buf[i]);
    }

    splits = 0;
    result_pptr = str_split(complex_str, complex_len, "*|*", 3, 0, &splits);
    printf(">>> Case 3: dynamic allocated result; with error code handling.\n");
    printf("str_split(%s, *|*): %d splits, err: %s\n", complex_str, splits, str_error(splits));
    if (NULL != result_pptr)
    {
        for (i = 0; i <= (size_t)splits; ++i) /* Or: (pptr = result_pptr, i = 0; NULL != *pptr; ++pptr, ++i) */
        {
            printf("    [%d]: %s\n", (int)i, result_pptr[i]);
        }
        str_split_destroy(result_pptr);
    }

    splits = str_split_to_fixed_buffer(complex_str, complex_len, "*|*", 3, buf_ptr, BUF_ITEM_COUNT, BUF_ITEM_SIZE);
    printf(">>> Case 4: fixed buffer on stack; with error code handling.\n");
    printf("str_split_to_fixed_buffer(%s, *|*): %d splits, err: %s\n", complex_str, splits, str_error(splits));
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i) /* Only if splits > 0 */
    {
        printf("    [%d]: %s\n", (int)i, buf[i]);
    }

    splits = 0;
    result_pptr = str_split(complex_str, complex_len, "\t\t", 2, 0, &splits);
    printf(">>> Case 5: delimiter not hit; dynamic allocated result; with error code handling.\n");
    printf("str_split(%s, \\t\\t): %d splits, err: %s\n", complex_str, splits, str_error(splits));
    if (NULL != result_pptr)
    {
        for (i = 0; i <= (size_t)splits; ++i) /* Or: (pptr = result_pptr, i = 0; NULL != *pptr; ++pptr, ++i) */
        {
            printf("    [%d]: %s\n", (int)i, result_pptr[i]);
        }
        str_split_destroy(result_pptr);
    }

    splits = str_split_to_fixed_buffer(complex_str, complex_len, "\t\t", 2, buf_ptr, BUF_ITEM_COUNT, BUF_ITEM_SIZE);
    printf(">>> Case 6: delimiter not hit; fixed buffer on stack; with error code handling.\n");
    printf("str_split_to_fixed_buffer(%s, \\t\\t): %d splits, err: %s\n", complex_str, splits, str_error(splits));
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i) /* Only if splits > 0 */
    {
        printf("    [%d]: %s\n", (int)i, buf[i]);
    }

    splits = 0;
    result_pptr = str_split("", 0, "\t", 1, 0, &splits);
    printf(">>> Case 7: empty target; dynamic allocated result; with error code handling.\n");
    printf("str_split('', \\t): %d splits, err: %s\n", splits, str_error(splits));
    if (NULL != result_pptr)
    {
        for (i = 0; i <= (size_t)splits; ++i) /* Or: (pptr = result_pptr, i = 0; NULL != *pptr; ++pptr, ++i) */
        {
            printf("    [%d]: %s\n", (int)i, result_pptr[i]);
        }
        str_split_destroy(result_pptr);
    }

    splits = str_split_to_fixed_buffer("", 0, "\t", 1, buf_ptr, BUF_ITEM_COUNT, BUF_ITEM_SIZE);
    printf(">>> Case 8: empty target; fixed buffer on stack; with error code handling.\n");
    printf("str_split_to_fixed_buffer('', \\t): %d splits, err: %s\n", splits, str_error(splits));
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i) /* Only if splits > 0 */
    {
        printf("    [%d]: %s\n", (int)i, buf[i]);
    }

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
 * >>> 2021-12-30, Man Hung-Coeng:
 *  01. Create.
 *
 * >>> 2022-01-06, Man Hung-Coeng:
 *  01. Rename this file from string_enhancements.c to string_supplements.c.
 *
 * >>> 2022-01-07, Man Hung-Coeng:
 *  01. Correct the typo of header inclusion.
 *
 * >>> 2022-01-08, Man Hung-Coeng:
 *  01. Fix the out-of-bounds error in str_error().
 *  02. Improve code style according to suggestions from cppcheck.
 *
 * >>> 2022-01-09, Man Hung-Coeng:
 *  01. Change str_split() to a function returning dynamically allocated memory
 *      result only, and add a new one named str_split_to_fixed_buffer()
 *      for storing the result in the static buffer specified by the parameter.
 *  02. Add str_split_destroy() for releasing the memory from str_split().
 *
 * >>> 2022-09-13, Man Hung-Coeng:
 *  01. Eliminate some warnings from compilers or VIM plugins.
 */

