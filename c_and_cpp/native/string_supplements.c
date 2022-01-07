/*
 * Supplements to string operation that ANSI C does not provide.
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

    if (error_code < -STR_ERR_END)
        return strerror(-error_code - STR_ERR_END);

    return S_ERRORS[-error_code - 1];
}

char** str_split(const char *str, size_t str_len, const char *delimiter, size_t delimiter_len,
    size_t *nullable_splits, char **nullable_result_holder, int *nullable_errcode)
{
    size_t splits = 0;
    char **pptr = nullable_result_holder;

    if (delimiter_len < str_len)
    {
        const size_t MAX_SPLITS = (NULL != nullable_splits && *nullable_splits > 0) ? *nullable_splits + 1 : (size_t)-1;
        size_t capacity = 1;
        size_t len = 0;
        const char *head = str;
        const char *tail = head;
        const char *STOP = str + str_len;
        char **tmp = NULL;

        if (NULL != nullable_errcode)
            *nullable_errcode = 0;

        while (++splits <= MAX_SPLITS)
        {
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

            if ((NULL == nullable_result_holder) && (NULL == pptr || splits > capacity))
            {
                capacity *= 2;
                tmp = (char **)realloc(pptr, sizeof(char *) * capacity);
                if (NULL == tmp)
                {
                    if (NULL != nullable_errcode)
                        *nullable_errcode = -STR_ERR_MEM_ALLOC;

                    break;
                }
                pptr = tmp;
            }

            len = (splits < MAX_SPLITS) ? (tail - head) : (STOP - head);
            if ((NULL == nullable_result_holder) && (NULL == (pptr[splits - 1] = malloc(len + 1))))
            {
                if (NULL != nullable_errcode)
                    *nullable_errcode = -STR_ERR_MEM_ALLOC;

                break;
            }
            memcpy(pptr[splits - 1], head, len);
            pptr[splits - 1][len] = '\0';

            tail += delimiter_len;
            head = tail;
        } /* while ((0 == MAX_SPLITS) || (++splits <= MAX_SPLITS)) */

        if (NULL == nullable_result_holder && capacity > splits - 1 &&
            (NULL != (tmp = realloc(pptr, sizeof(char *) * (splits - 1)))))
        {
            pptr = tmp;
        }
    } /* delimiter_len < str_len */
    else
    {
        if (NULL != nullable_errcode)
            *nullable_errcode = -STR_ERR_SPLITTING_SKIPPED;
    }

    if (NULL != nullable_splits)
        *nullable_splits = (delimiter_len < str_len) ? (splits - 2) : 0;

    return pptr;
}

#ifdef TEST

#include <stdio.h>

int main(int argc, char **argv)
{
    const char *simple_str = "/aa//bb/cc//";
    size_t simple_len = strlen(simple_str);
    const char *complex_str = "*|*a*|**|*bb*|*ccc*|dd*|*e*|*ff*|*ggg*|*";
    size_t complex_len = strlen(complex_str);
    size_t splits = 0;
    char **result_pptr = NULL;
    char buf[8][32] = { 0 };
    char *buf_ptr[8]; /* TODO: Any easier initialization ?? */
    size_t i = 0;
    int err = 0;

    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i)
    {
        buf_ptr[i] = buf[i];
    }

    result_pptr = str_split(simple_str, simple_len, "/", 1, &splits, NULL, NULL);
    printf(">>> Case 1: dynamic allocated result; ignored error code.\n");
    printf("str_split(%s, /): %d splits\n", simple_str, (int)splits);
    if (NULL != result_pptr)
    {
        for (i = 0; i <= splits; ++i)
        {
            printf("    [%d]: %s\n", (int)i, result_pptr[i]);
            free(result_pptr[i]);
        }
        free(result_pptr);
    }

    str_split(simple_str, simple_len, "/", 1, NULL, buf_ptr, NULL);
    printf(">>> Case 2: buffer on stack; ignored error code.\n");
    printf("str_split(%s, /):\n", simple_str);
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i)
    {
        printf("    [%d]: %s\n", (int)i, buf[i]);
    }

    splits = 0;
    result_pptr = str_split(complex_str, complex_len, "*|*", 3, &splits, NULL, &err);
    printf(">>> Case 3: dynamic allocated result; with error code handling.\n");
    printf("str_split(%s, *|*): %d splits, err: %s\n", complex_str, (int)splits, str_error(err));
    if (NULL != result_pptr)
    {
        for (i = 0; i <= splits; ++i)
        {
            printf("    [%d]: %s\n", (int)i, result_pptr[i]);
            free(result_pptr[i]);
        }
        free(result_pptr);
    }

    splits = sizeof(buf_ptr) / sizeof(char *) - 1;
    str_split(complex_str, complex_len, "*|*", 3, &splits, buf_ptr, &err);
    printf(">>> Case 4: with maximum split times; buffer on stack; with error code handling.\n");
    printf("str_split(%s, *|*): %d splits, err: %s\n", complex_str, (int)splits, str_error(err));
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i)
    {
        printf("    [%d]: %s\n", (int)i, buf[i]);
    }

    splits = 0;
    result_pptr = str_split(complex_str, complex_len, "\t\t", 2, &splits, NULL, &err);
    printf(">>> Case 5: delimiter not hit; dynamic allocated result; with error code handling.\n");
    printf("str_split(%s, \\t\\t): %d splits, err: %s\n", complex_str, (int)splits, str_error(err));
    if (NULL != result_pptr)
    {
        for (i = 0; i <= splits; ++i)
        {
            printf("    [%d]: %s\n", (int)i, result_pptr[i]);
            free(result_pptr[i]);
        }
        free(result_pptr);
    }

    splits = sizeof(buf_ptr) / sizeof(char *) - 1;
    str_split(complex_str, complex_len, "\t\t", 2, &splits, buf_ptr, &err);
    printf(">>> Case 6: delimiter not hit; with maximum split times; buffer on stack; with error code handling.\n");
    printf("str_split(%s, \\t\\t): %d splits, err: %s\n", complex_str, (int)splits, str_error(err));
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i)
    {
        printf("    [%d]: %s\n", (int)i, buf[i]);
    }

    splits = 0;
    result_pptr = str_split("", 0, "\t", 1, &splits, NULL, &err);
    printf(">>> Case 7: empty target; dynamic allocated result; with error code handling.\n");
    printf("str_split('', \\t): %d splits, err: %s\n", (int)splits, str_error(err));
    if (NULL != result_pptr)
    {
        for (i = 0; i <= splits; ++i)
        {
            printf("    [%d]: %s\n", (int)i, result_pptr[i]);
            free(result_pptr[i]);
        }
        free(result_pptr);
    }

    splits = sizeof(buf_ptr) / sizeof(char *) - 1;
    str_split("", 0, "\t", 1, &splits, buf_ptr, &err);
    printf(">>> Case 8: empty target; with maximum split times; buffer on stack; with error code handling.\n");
    printf("str_split('', \\t): %d splits, err: %s\n", (int)splits, str_error(err));
    for (i = 0; i < sizeof(buf_ptr) / sizeof(char *); ++i)
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
 */


