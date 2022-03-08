/*
 * Data serialization, deserialization, etc. for communication purpose.
 *
 * Copyright (c) 2022 Man Hung-Coeng <udc577@126.com>
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

#include "communication_protocol.h"

#include <stddef.h> /* For ptrdiff_t. */
#include <stdbool.h> /* For bool, true and false. */
#include <ctype.h> /* For isgraph(). */
#include <string.h> /* For memset(). */
#include <stdlib.h> /* For exit(). */

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    COMMPROTO_ERR_UNKNOWN = 1
    , COMMPROTO_ERR_NOT_IMPLEMENTED
    , COMMPROTO_ERR_MEM_ALLOC
    , COMMPROTO_ERR_ZERO_LENGTH
    , COMMPROTO_ERR_STRING_TOO_LONG
    , COMMPROTO_ERR_NOT_INITIALIZED
    , COMMPROTO_ERR_UNKNOWN_FIELD_TYPE
    , COMMPROTO_ERR_PACKET_TOO_BIG
    , COMMPROTO_ERR_WRONG_META_DATA
    , COMMPROTO_ERR_META_ARRAY_LENGTH_MISSING
    , COMMPROTO_ERR_STRUCT_PTR_EXCEEDS
    , COMMPROTO_ERR_INCOMPLETE_BUF_CONTENTS

    , COMMPROTO_ERR_END /* NOTE: All error codes should be defined ahead of this. */
};

static const char* const S_ERRORS[] = {
    "Unknown error"
    , "Not implemented"
    , "Failed to allocate memory"
    , "Zero length"
    , "String too long"
    , "Not initialized"
    , "Unknown field type"
    , "Packet too big"
    , "Wrong meta data"
    , "Meta array length missing"
    , "Structure pointer exceeds"
    , "Incomplete buffer contents"
};

const char* commproto_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    if (error_code <= -COMMPROTO_ERR_END)
        return strerror(-error_code - COMMPROTO_ERR_END);

    return S_ERRORS[-error_code - 1];
}

static bool s_is_initialized = false;

int commproto_init(void)
{
    union
    {
        char c2[2];
        int16_t i16;
    } byte_order_var;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    const char EXPECTED_C0 = 0x34;
#else
    const char EXPECTED_C0 = 0x12;
#endif

    byte_order_var.i16 = 0x1234;

    if (EXPECTED_C0 != byte_order_var.c2[0])
    {
        fprintf(stderr, "*** Byte order mismatched due to incorrect endianness macro definition(s)!\n");
        fprintf(stderr, __FILE__ ":%d: Please redefine one or more of macro __BYTE_ORDER__,"
            " __ORDER_BIG_ENDIAN__ and __ORDER_LITTLE_ENDIAN__, then recompile this file!\n", __LINE__);
        exit(EXIT_FAILURE);
    }

    s_is_initialized = true;

    return 0;
}

#define COMMPROTO_EXPAND_BUFSIZE(old_size)  (((old_size) < 1024) ? ((old_size) * 2) : (1024 * ((old_size) / 1024 + 1)))

#ifndef COMMPROTO_MAX_BUFSIZE
#define COMMPROTO_MAX_BUFSIZE               1024 * 32
#endif

#if 0 != COMMPROTO_MAX_BUFSIZE % 1024
#error COMMPROTO_MAX_BUFSIZE is not multiples of 1024!
#endif

#if COMMPROTO_MAX_BUFSIZE < 1024
#error COMMPROTO_MAX_BUFSIZE must be greater than or equal to 1024!
#endif

#if COMMPROTO_MAX_BUFSIZE > 1024 * 63
#error COMMPROTO_MAX_BUFSIZE must be less than or equal to 1024 * 63!
#endif

#ifndef COMMPROTO_INITIAL_BUFSIZE
#define COMMPROTO_INITIAL_BUFSIZE           128
#endif

#if (COMMPROTO_INITIAL_BUFSIZE != 4) && (COMMPROTO_INITIAL_BUFSIZE != 8) && \
    (COMMPROTO_INITIAL_BUFSIZE != 16) && (COMMPROTO_INITIAL_BUFSIZE != 32) && \
    (COMMPROTO_INITIAL_BUFSIZE != 64) && (COMMPROTO_INITIAL_BUFSIZE != 128)
#error COMMPROTO_INITIAL_BUFSIZE must be 4, 8, 16, 32, 64, 128!
#endif

static int16_t calc_struct_size_or_move_meta_ptr(int16_t struct_field_count, const uint8_t *meta_ptr,
    uint16_t meta_len, uint8_t **nullable_meta_pptr)
{
    const uint8_t *meta_start_ptr = meta_ptr;
    int16_t result = 0;
    int16_t i = 0;

    for (; i < struct_field_count; ++i)
    {
        int8_t type = *meta_ptr;

        if (type > 0 && type <= COMMPROTO_FLOAT64)
        {
            result += (type % 10);
            meta_ptr += sizeof(int8_t);
            if (meta_ptr - meta_start_ptr >= meta_len)
                break;
            continue;
        }

        if (COMMPROTO_ARRAY_LEN == type)
        {
            result += sizeof(int16_t);
            meta_ptr += sizeof(int8_t);
            if (meta_ptr - meta_start_ptr >= meta_len)
                break;
            continue;
        }

        if (type <= COMMPROTO_FLOAT64_FIXED_ARRAY)
        {
            result += ((type % 10) * (*((int16_t *)(meta_ptr + sizeof(int8_t)))));
            meta_ptr += (sizeof(int8_t) + sizeof(int16_t));
            if (meta_ptr - meta_start_ptr >= meta_len)
                break;
            continue;
        }

        if (type <= COMMPROTO_FLOAT64_DYNAMIC_ARRAY)
        {
            result += sizeof(ptrdiff_t);
            meta_ptr += sizeof(int8_t);
            if (meta_ptr - meta_start_ptr >= meta_len)
                break;
            continue;
        }

        if (COMMPROTO_STRUCT_FIXED_ARRAY == type)
        {
            uint8_t *sub_meta_ptr = (uint8_t *)meta_ptr + sizeof(int8_t) + sizeof(int16_t) * 2;
            int16_t sub_struct_count = *((int16_t *)(meta_ptr + sizeof(int8_t) + sizeof(int16_t)));
            int16_t sub_struct_size = calc_struct_size_or_move_meta_ptr(
                *((int16_t *)(meta_ptr + sizeof(int8_t))), sub_meta_ptr,
                meta_len, &sub_meta_ptr
            );

            if (sub_struct_size < 0)
            {
                if (NULL != nullable_meta_pptr)
                    *nullable_meta_pptr = sub_meta_ptr;

                return sub_struct_size;
            }

            result += (sub_struct_size * sub_struct_count);

            meta_ptr = sub_meta_ptr;
            if (meta_ptr - meta_start_ptr >= meta_len)
                break;

            continue;
        }

        if (COMMPROTO_STRUCT_DYNAMIC_ARRAY == type)
        {
            uint8_t *sub_meta_ptr = (uint8_t *)meta_ptr + sizeof(int8_t) + sizeof(int16_t);
            int16_t sub_struct_size = calc_struct_size_or_move_meta_ptr(
                *((int16_t *)(meta_ptr + sizeof(int8_t))), sub_meta_ptr,
                meta_len, &sub_meta_ptr
            );

            if (sub_struct_size < 0)
            {
                if (NULL != nullable_meta_pptr)
                    *nullable_meta_pptr = sub_meta_ptr;

                return sub_struct_size;
            }

            result += sizeof(ptrdiff_t);

            meta_ptr = sub_meta_ptr;
            if (meta_ptr - meta_start_ptr >= meta_len)
                break;

            continue;
        }

        if (NULL != nullable_meta_pptr)
            *nullable_meta_pptr = (uint8_t *)meta_ptr;

        return -COMMPROTO_ERR_WRONG_META_DATA;
    } /* for (; i < struct_field_count; ++i) */

    if (NULL != nullable_meta_pptr)
        *nullable_meta_pptr = (uint8_t *)meta_ptr;

    COMMPROTO_DPRINT("struct size: %d\n", result);

    return result;
}

static int general_serialization(int16_t fields, int16_t loops, const uint8_t *meta_start_ptr,
    uint16_t meta_len, uint8_t **meta_pptr, uint8_t **struct_pptr,
    bool can_have_inner_struct, bool is_static_buf, uint16_t max_buf_len,
    uint8_t **buf_pptr, uint16_t *buf_capacity_ptr, uint16_t *handled_len_ptr)
{
    int16_t simple_array_len = -1;
    int16_t struct_array_len = -1;
    int16_t struct_field_count = -1;
    uint8_t *inner_struct_ptr = NULL;
    uint8_t *meta_ptr_per_round = *meta_pptr;
#if defined(COMMPROTO_DEBUG) || defined(TEST)
    uint8_t *struct_ptr_start = *struct_pptr;
#endif
    uint8_t *new_buf = NULL;
    int err = 0;
    int16_t loop = 1;

    for (; loop <= loops && err >= 0; ++loop)
    {
        int16_t field = 1;

        while (err >= 0)
        {
            int8_t type = **meta_pptr;
            bool is_dynamic_struct_array = (COMMPROTO_STRUCT_DYNAMIC_ARRAY == type);
            uint16_t meta_offset = sizeof(int8_t);
            int32_t data_offset = 0;

            switch (type) /* Step 1: Determine offsets and array lengths. */
            {
            case COMMPROTO_INT8:
            case COMMPROTO_INT16:
            case COMMPROTO_INT32:
            case COMMPROTO_INT64:
            case COMMPROTO_FLOAT32:
            case COMMPROTO_FLOAT64:
                data_offset += (type % 10);
                break;

            case COMMPROTO_INT8_FIXED_ARRAY:
            case COMMPROTO_INT16_FIXED_ARRAY:
            case COMMPROTO_INT32_FIXED_ARRAY:
            case COMMPROTO_INT64_FIXED_ARRAY:
            case COMMPROTO_FLOAT32_FIXED_ARRAY:
            case COMMPROTO_FLOAT64_FIXED_ARRAY:
                simple_array_len = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                data_offset += ((type % 10) * simple_array_len);
                meta_offset += sizeof(int16_t);
                break;

            case COMMPROTO_ARRAY_LEN:
                simple_array_len = struct_array_len = *((int16_t *)*struct_pptr);
                data_offset += sizeof(int16_t);
                break;

            case COMMPROTO_INT8_DYNAMIC_ARRAY:
            case COMMPROTO_INT16_DYNAMIC_ARRAY:
            case COMMPROTO_INT32_DYNAMIC_ARRAY:
            case COMMPROTO_INT64_DYNAMIC_ARRAY:
            case COMMPROTO_FLOAT32_DYNAMIC_ARRAY:
            case COMMPROTO_FLOAT64_DYNAMIC_ARRAY:
                if (simple_array_len < 0)
                {
                    err = -COMMPROTO_ERR_META_ARRAY_LENGTH_MISSING;
                    continue;
                }
                data_offset += ((type % 10) * simple_array_len);
                break;

            case COMMPROTO_STRUCT_FIXED_ARRAY:
                if (!can_have_inner_struct)
                {
                    err = -COMMPROTO_ERR_WRONG_META_DATA;
                    continue;
                }
                struct_field_count = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                struct_array_len = *((int16_t *)(*meta_pptr + sizeof(int8_t) + sizeof(int16_t)));
                meta_offset += (sizeof(int16_t) * 2);
                break;

            case COMMPROTO_STRUCT_DYNAMIC_ARRAY:
                if (struct_array_len < 0 || !can_have_inner_struct)
                {
                    err = (struct_array_len < 0) ? -COMMPROTO_ERR_META_ARRAY_LENGTH_MISSING
                        : -COMMPROTO_ERR_WRONG_META_DATA;
                    continue;
                }
                struct_field_count = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                meta_offset += sizeof(int16_t);
                break;

            default:
                err = -COMMPROTO_ERR_UNKNOWN_FIELD_TYPE;
                continue;
            } /* switch (type) */

            *meta_pptr += meta_offset;

            if (*handled_len_ptr + data_offset > *buf_capacity_ptr) /* Step 2: Expand buffer if needed. */
            {
                if (is_static_buf)
                {
                    err = -COMMPROTO_ERR_PACKET_TOO_BIG;
                    continue;
                }

                *buf_capacity_ptr = COMMPROTO_EXPAND_BUFSIZE(*buf_capacity_ptr);
                if (*buf_capacity_ptr > max_buf_len || *handled_len_ptr + data_offset > *buf_capacity_ptr)
                {
                    err = -COMMPROTO_ERR_PACKET_TOO_BIG;
                    continue;
                }

                if (NULL == (new_buf = (uint8_t *)realloc(*buf_pptr, *buf_capacity_ptr)))
                {
                    err = -COMMPROTO_ERR_MEM_ALLOC;
                    continue;
                }
                *buf_pptr = new_buf;
                COMMPROTO_DPRINT("Expanded new_buf addr = %p, buf_capacity = %u\n", new_buf, *buf_capacity_ptr);
            }

            if (type < COMMPROTO_STRUCT_FIXED_ARRAY)
                *handled_len_ptr += data_offset;

            switch (type) /* Step 3: Do assignments. */
            {
            case COMMPROTO_INT8:
            case COMMPROTO_INT8_FIXED_ARRAY:
            case COMMPROTO_INT8_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT8_ARRAY(((COMMPROTO_INT8 == type) ? 1 : simple_array_len),
                    ((COMMPROTO_INT8_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr),
                    *buf_pptr + *handled_len_ptr - data_offset);
                *struct_pptr += ((COMMPROTO_INT8_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_ARRAY_LEN:
            case COMMPROTO_INT16:
            case COMMPROTO_INT16_FIXED_ARRAY:
            case COMMPROTO_INT16_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT16_ARRAY(((COMMPROTO_INT16 == type || COMMPROTO_ARRAY_LEN == type) ? 1 : simple_array_len),
                    ((COMMPROTO_INT16_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr),
                    *buf_pptr + *handled_len_ptr - data_offset);
                *struct_pptr += ((COMMPROTO_INT16_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_INT32:
            case COMMPROTO_INT32_FIXED_ARRAY:
            case COMMPROTO_INT32_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT32_ARRAY(((COMMPROTO_INT32 == type) ? 1 : simple_array_len),
                    ((COMMPROTO_INT32_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr),
                    *buf_pptr + *handled_len_ptr - data_offset);
                *struct_pptr += ((COMMPROTO_INT32_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_INT64:
            case COMMPROTO_INT64_FIXED_ARRAY:
            case COMMPROTO_INT64_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT64_ARRAY(((COMMPROTO_INT64 == type) ? 1 : simple_array_len),
                    ((COMMPROTO_INT64_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr),
                    *buf_pptr + *handled_len_ptr - data_offset);
                *struct_pptr += ((COMMPROTO_INT64_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_FLOAT32:
            case COMMPROTO_FLOAT32_FIXED_ARRAY:
            case COMMPROTO_FLOAT32_DYNAMIC_ARRAY:
                COMMPROTO_SET_FLOAT32_ARRAY(((COMMPROTO_FLOAT32 == type) ? 1 : simple_array_len),
                    ((COMMPROTO_FLOAT32_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr),
                    *buf_pptr + *handled_len_ptr - data_offset);
                *struct_pptr += ((COMMPROTO_FLOAT32_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_FLOAT64:
            case COMMPROTO_FLOAT64_FIXED_ARRAY:
            case COMMPROTO_FLOAT64_DYNAMIC_ARRAY:
                COMMPROTO_SET_FLOAT64_ARRAY(((COMMPROTO_FLOAT64 == type) ? 1 : simple_array_len),
                    ((COMMPROTO_FLOAT64_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr),
                    *buf_pptr + *handled_len_ptr - data_offset);
                *struct_pptr += ((COMMPROTO_FLOAT64_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            default:
                inner_struct_ptr = (is_dynamic_struct_array ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : NULL);
                err = (0 == struct_field_count || 0 == struct_array_len) ? 0
                    : general_serialization(struct_field_count, struct_array_len, meta_start_ptr,
                        meta_len, meta_pptr, (is_dynamic_struct_array ? &inner_struct_ptr : struct_pptr),
                        /* can_have_inner_struct = */false, is_static_buf, max_buf_len,
                        buf_pptr, buf_capacity_ptr, handled_len_ptr);
                if (is_dynamic_struct_array && err >= 0)
                {
                    *struct_pptr += sizeof(ptrdiff_t);
                    if (0 == struct_array_len && struct_field_count > 0)
                        calc_struct_size_or_move_meta_ptr(struct_field_count, *meta_pptr, meta_len, meta_pptr);
                }
                break;
            } /* switch (type) */

            COMMPROTO_DPRINT("loop[%d/%d] Serialization: type: %d, handled len: %d, meta offset: %d,"
                " struct offset: %d\n", loop, loops, type, *handled_len_ptr, (int)(*meta_pptr - meta_start_ptr),
                (int)(*struct_pptr - struct_ptr_start));

            if (*meta_pptr - meta_start_ptr >= meta_len || ++field > fields)
                break;
        } /* while (err >= 0) */

        if (0 == err && loop < loops)
            *meta_pptr = meta_ptr_per_round;
    } /* for (; loop <= loops && err >= 0; ++loop) */

    return err;
}

uint8_t* commproto_serialize(const uint8_t *struct_meta_data, uint16_t meta_len, const void *one_byte_aligned_struct,
    uint8_t *nullable_buf, uint16_t buf_len, uint16_t *nullable_handled_len, int *nullable_error_code)
{
    uint8_t *meta_ptr = (uint8_t *)struct_meta_data;
    uint8_t *struct_ptr = (uint8_t *)one_byte_aligned_struct;
    bool is_static_buf = (NULL != nullable_buf);
    const uint16_t MAX_BUF_LEN = is_static_buf ? buf_len : COMMPROTO_MAX_BUFSIZE;
    uint16_t result_len = 0;
    uint16_t buf_capacity = is_static_buf ? buf_len : COMMPROTO_INITIAL_BUFSIZE;
    uint8_t *buf_ptr = is_static_buf ? nullable_buf : (uint8_t *)malloc(buf_capacity);
    uint8_t *new_buf = NULL;
    int err = s_is_initialized
        ? (
            is_static_buf
            ? ((0 == buf_len) ? -COMMPROTO_ERR_ZERO_LENGTH : 0)
            : ((NULL == buf_ptr) ? -COMMPROTO_ERR_MEM_ALLOC : 0)
        )
        : -COMMPROTO_ERR_NOT_INITIALIZED;

    if (err < 0)
        goto SERIALIZE_END;

    if ((err = general_serialization(/* fields = */0xffff / 2, /* loops = */1, struct_meta_data,
        meta_len, &meta_ptr, &struct_ptr,
        /* can_have_inner_struct = */true, is_static_buf, MAX_BUF_LEN,
        &buf_ptr, &buf_capacity, &result_len)) < 0)
    {
        goto SERIALIZE_END;
    }

    if ((!is_static_buf) && result_len < buf_capacity && NULL != (new_buf = (uint8_t *)realloc(buf_ptr, result_len)))
        buf_ptr = new_buf;

SERIALIZE_END:

    if (NULL != nullable_handled_len)
        *nullable_handled_len = result_len;

    if (NULL != nullable_error_code)
        *nullable_error_code = err;

    return buf_ptr;
}

static int general_deserialization(int16_t fields, int16_t loops, const uint8_t *meta_start_ptr,
    uint16_t meta_len, uint8_t **meta_pptr, const uint8_t *buf_ptr,
    uint16_t buf_len, uint8_t **struct_pptr, int16_t struct_size,
    bool can_have_inner_struct, uint16_t *handled_len_ptr)
{
    int16_t simple_array_len = -1;
    int16_t struct_array_len = -1;
    int16_t struct_field_count = -1;
    uint8_t *inner_struct_ptr = NULL;
    bool should_expand_dynamic_array = false;
    uint8_t *meta_ptr_per_round = *meta_pptr;
    uint8_t *struct_ptr_start = *struct_pptr;
    int err = 0;
    int16_t loop = 1;

    for (; loop <= loops && err >= 0; ++loop)
    {
        int16_t field = 1;

        while (err >= 0 && *handled_len_ptr < buf_len)
        {
            int8_t type = **meta_pptr;
            bool is_dynamic_struct_array = (COMMPROTO_STRUCT_DYNAMIC_ARRAY == type);
            int16_t dynamic_array_size = 0;
            uint16_t meta_offset = sizeof(int8_t);
            int32_t data_offset = 0;

            switch (type) /* Step 1: Determine offsets and array lengths. */
            {
            case COMMPROTO_INT8:
            case COMMPROTO_INT16:
            case COMMPROTO_INT32:
            case COMMPROTO_INT64:
            case COMMPROTO_FLOAT32:
            case COMMPROTO_FLOAT64:
                data_offset += (type % 10);
                break;

            case COMMPROTO_INT8_FIXED_ARRAY:
            case COMMPROTO_INT16_FIXED_ARRAY:
            case COMMPROTO_INT32_FIXED_ARRAY:
            case COMMPROTO_INT64_FIXED_ARRAY:
            case COMMPROTO_FLOAT32_FIXED_ARRAY:
            case COMMPROTO_FLOAT64_FIXED_ARRAY:
                simple_array_len = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                data_offset += ((type % 10) * simple_array_len);
                meta_offset += sizeof(int16_t);
                break;

            case COMMPROTO_ARRAY_LEN:
                data_offset += sizeof(int16_t);
                break;

            case COMMPROTO_INT8_DYNAMIC_ARRAY:
            case COMMPROTO_INT16_DYNAMIC_ARRAY:
            case COMMPROTO_INT32_DYNAMIC_ARRAY:
            case COMMPROTO_INT64_DYNAMIC_ARRAY:
            case COMMPROTO_FLOAT32_DYNAMIC_ARRAY:
            case COMMPROTO_FLOAT64_DYNAMIC_ARRAY:
                if (simple_array_len < 0)
                {
                    err = -COMMPROTO_ERR_META_ARRAY_LENGTH_MISSING;
                    continue;
                }
                dynamic_array_size = data_offset = ((type % 10) * simple_array_len);
                break;

            case COMMPROTO_STRUCT_FIXED_ARRAY:
                if (!can_have_inner_struct)
                {
                    err = -COMMPROTO_ERR_WRONG_META_DATA;
                    continue;
                }
                struct_field_count = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                struct_array_len = *((int16_t *)(*meta_pptr + sizeof(int8_t) + sizeof(int16_t)));
                meta_offset += (sizeof(int16_t) * 2);
                break;

            case COMMPROTO_STRUCT_DYNAMIC_ARRAY:
                if (struct_array_len < 0 || !can_have_inner_struct)
                {
                    err = (struct_array_len < 0) ? -COMMPROTO_ERR_META_ARRAY_LENGTH_MISSING
                        : -COMMPROTO_ERR_WRONG_META_DATA;
                    continue;
                }
                struct_field_count = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                meta_offset += sizeof(int16_t);
                dynamic_array_size = calc_struct_size_or_move_meta_ptr(struct_field_count,
                    *meta_pptr + meta_offset, meta_len, NULL) * struct_array_len;
                break;

            default:
                err = -COMMPROTO_ERR_UNKNOWN_FIELD_TYPE;
                continue;
            } /* switch (type) */

            *meta_pptr += meta_offset;

            if (*handled_len_ptr + data_offset > buf_len ||
                dynamic_array_size < 0 || *handled_len_ptr + dynamic_array_size > buf_len)
            {
                err = (dynamic_array_size < 0) ? dynamic_array_size : -COMMPROTO_ERR_INCOMPLETE_BUF_CONTENTS;
                continue;
            }

            /* Step 2: Allocate memory for current dynamic array if needed. */
            if (type >= COMMPROTO_INT8_DYNAMIC_ARRAY && COMMPROTO_STRUCT_FIXED_ARRAY != type
                && dynamic_array_size > 0
                && (should_expand_dynamic_array || NULL == (char *)**(ptrdiff_t **)struct_pptr))
            {
                char *new_array = (char *)realloc((char *)**(ptrdiff_t **)struct_pptr, dynamic_array_size);

                if (NULL == new_array)
                {
                    err = -COMMPROTO_ERR_MEM_ALLOC;
                    continue;
                }
                **(ptrdiff_t **)struct_pptr = (ptrdiff_t)new_array;
            }

            if (type < COMMPROTO_STRUCT_FIXED_ARRAY)
                *handled_len_ptr += data_offset;

            switch (type) /* Step 3: Do assignments. */
            {
            case COMMPROTO_INT8:
            case COMMPROTO_INT8_FIXED_ARRAY:
            case COMMPROTO_INT8_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT8_ARRAY(((COMMPROTO_INT8 == type) ? 1 : simple_array_len),
                    buf_ptr + *handled_len_ptr - data_offset,
                    ((COMMPROTO_INT8_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr));
                *struct_pptr += ((COMMPROTO_INT8_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_INT16:
            case COMMPROTO_INT16_FIXED_ARRAY:
            case COMMPROTO_INT16_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT16_ARRAY(((COMMPROTO_INT16 == type) ? 1 : simple_array_len),
                    buf_ptr + *handled_len_ptr - data_offset,
                    ((COMMPROTO_INT16_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr));
                *struct_pptr += ((COMMPROTO_INT16_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_INT32:
            case COMMPROTO_INT32_FIXED_ARRAY:
            case COMMPROTO_INT32_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT32_ARRAY(((COMMPROTO_INT32 == type) ? 1 : simple_array_len),
                    buf_ptr + *handled_len_ptr - data_offset,
                    ((COMMPROTO_INT32_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr));
                *struct_pptr += ((COMMPROTO_INT32_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_INT64:
            case COMMPROTO_INT64_FIXED_ARRAY:
            case COMMPROTO_INT64_DYNAMIC_ARRAY:
                COMMPROTO_SET_INT64_ARRAY(((COMMPROTO_INT64 == type) ? 1 : simple_array_len),
                    buf_ptr + *handled_len_ptr - data_offset,
                    ((COMMPROTO_INT64_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr));
                *struct_pptr += ((COMMPROTO_INT64_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_FLOAT32:
            case COMMPROTO_FLOAT32_FIXED_ARRAY:
            case COMMPROTO_FLOAT32_DYNAMIC_ARRAY:
                COMMPROTO_SET_FLOAT32_ARRAY(((COMMPROTO_FLOAT32 == type) ? 1 : simple_array_len),
                    buf_ptr + *handled_len_ptr - data_offset,
                    ((COMMPROTO_FLOAT32_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr));
                *struct_pptr += ((COMMPROTO_FLOAT32_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_FLOAT64:
            case COMMPROTO_FLOAT64_FIXED_ARRAY:
            case COMMPROTO_FLOAT64_DYNAMIC_ARRAY:
                COMMPROTO_SET_FLOAT64_ARRAY(((COMMPROTO_FLOAT64 == type) ? 1 : simple_array_len),
                    buf_ptr + *handled_len_ptr - data_offset,
                    ((COMMPROTO_FLOAT64_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr));
                *struct_pptr += ((COMMPROTO_FLOAT64_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset);
                break;

            case COMMPROTO_ARRAY_LEN:
                COMMPROTO_SET_INT16(buf_ptr + *handled_len_ptr - data_offset, &simple_array_len);
                should_expand_dynamic_array = (simple_array_len > *((int16_t *)*struct_pptr));
                *((int16_t *)*struct_pptr) = struct_array_len = simple_array_len;
                *struct_pptr += data_offset;
                break;

            default:
                inner_struct_ptr = (is_dynamic_struct_array ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : NULL);
                err = (0 == struct_field_count || 0 == struct_array_len) ? 0
                    : general_deserialization(struct_field_count, struct_array_len, meta_start_ptr,
                        meta_len, meta_pptr, buf_ptr,
                        buf_len, (is_dynamic_struct_array ? &inner_struct_ptr : struct_pptr), struct_size,
                        /* can_have_inner_struct = */false, handled_len_ptr);
                if (is_dynamic_struct_array && err >= 0)
                {
                    *struct_pptr += sizeof(ptrdiff_t);
                    if (0 == struct_array_len && struct_field_count > 0)
                        calc_struct_size_or_move_meta_ptr(struct_field_count, *meta_pptr, meta_len, meta_pptr);
                }
                break;
            } /* switch (type) */

            COMMPROTO_DPRINT("loop[%d/%d] Deserialization: type: %d, handled len: %d, meta offset: %d,"
                " struct offset: %d\n", loop, loops, type, *handled_len_ptr, (int)(*meta_pptr - meta_start_ptr),
                (int)(*struct_pptr - struct_ptr_start));

            if (*struct_pptr - struct_ptr_start > struct_size)
            {
                err = -COMMPROTO_ERR_STRUCT_PTR_EXCEEDS;
                continue;
            }

            if (*meta_pptr - meta_start_ptr >= meta_len || ++field > fields)
                break;
        } /* while (err >= 0 && *handled_len_ptr < buf_len) */

        if (0 == err && loop < loops)
            *meta_pptr = meta_ptr_per_round;
    } /* for (; loop <= loops && err >= 0; ++loop) */

    return err;
}

int commproto_parse(const uint8_t *struct_meta_data, uint16_t meta_len, const uint8_t *buf, uint16_t buf_len,
    void *one_byte_aligned_struct, uint16_t *nullable_handled_len)
{
    uint8_t *meta_ptr = (uint8_t *)struct_meta_data;
    uint8_t *struct_ptr = (uint8_t *)one_byte_aligned_struct;
    const int16_t STRUCT_SIZE = calc_struct_size_or_move_meta_ptr(0xffff / 2, meta_ptr, meta_len, NULL);
    uint16_t parsed_len = 0;
    int err = s_is_initialized
        ? (
            (0 == buf_len)
            ? -COMMPROTO_ERR_ZERO_LENGTH
            : general_deserialization(
                /* fields = */0xffff / 2, /* loops = */1, struct_meta_data,
                meta_len, &meta_ptr, buf,
                buf_len, &struct_ptr, STRUCT_SIZE,
                /* can_have_inner_struct = */true, &parsed_len
            )
        )
        : -COMMPROTO_ERR_NOT_INITIALIZED;

    if (NULL != nullable_handled_len)
        *nullable_handled_len = parsed_len;

    return err;
}

void commproto_dump_buffer(const uint8_t *buf, uint16_t size, FILE *nullable_stream, char *nullable_holder)
{
    char hex1[3 * 8 + 1] = { 0 };
    char hex2[3 * 8 + 1] = { 0 };
    char str1[8 + 1] = { 0 };
    char str2[8 + 1] = { 0 };
    uint16_t i = 0;

    for (; i < size; ++i)
    {
        uint16_t remainder = i % 16;

        if (remainder < 8)
        {
            hex1[3 * remainder] = ' ';
            sprintf(hex1 + 3 * remainder + 1, "%02x", buf[i]);

            str1[remainder] = isgraph(buf[i]) ? buf[i] : '.';
        }
        else
        {
            hex2[3 * (remainder - 8)] = ' ';
            sprintf(hex2 + 3 * (remainder - 8) + 1, "%02x", buf[i]);

            str2[remainder - 8] = isgraph(buf[i]) ? buf[i] : '.';
        }

        if (15 != remainder && size - 1 != i)
            continue;

        if (remainder < 8)
        {
            memset(hex1 + 3 * (remainder + 1), ' ', sizeof(hex1) - 1 - 3 * (remainder + 1));
            memset(hex2, ' ', sizeof(hex2) - 1);
            memset(str1 + remainder + 1, ' ', sizeof(str1) - 1 - (remainder + 1));
            memset(str2, ' ', sizeof(str2) - 1);
        }
        else
        {
            memset(hex2 + 3 * ((remainder - 8) + 1), ' ', sizeof(hex2) - 1 - 3 * ((remainder - 8) + 1));
            memset(str2 + (remainder - 8) + 1, ' ', sizeof(str2) - 1 - ((remainder - 8) + 1));
        }

        if (NULL != nullable_stream)
        {
            fprintf(nullable_stream, "%05d", i / 16);
            fprintf(nullable_stream, "%s", hex1);
            fprintf(nullable_stream, " ");
            fprintf(nullable_stream, "%s", hex2);
            fprintf(nullable_stream, "   ");
            fprintf(nullable_stream, "%s", str1);
            fprintf(nullable_stream, " ");
            fprintf(nullable_stream, "%s", str2);
            fprintf(nullable_stream, "\n");
        }

        if (NULL != nullable_holder)
        {
            sprintf(nullable_holder, "%05d", i / 16);
            nullable_holder += 5;
            memcpy(nullable_holder, hex1, sizeof(hex1) - 1);
            nullable_holder += (sizeof(hex1) - 1);
            *nullable_holder = ' ';
            nullable_holder += 1;
            memcpy(nullable_holder, hex2, sizeof(hex2) - 1);
            nullable_holder += (sizeof(hex2) - 1);
            memcpy(nullable_holder, "   ", 3);
            nullable_holder += 3;
            memcpy(nullable_holder, str1, sizeof(str1) - 1);
            nullable_holder += (sizeof(str1) - 1);
            *nullable_holder = ' ';
            nullable_holder += 1;
            memcpy(nullable_holder, str2, sizeof(str2) - 1);
            nullable_holder += (sizeof(str2) - 1);
            *nullable_holder = '\n';
            nullable_holder += 1;
        }
    } /* for (; i < size; ++i) */

    if (NULL != nullable_holder)
        *nullable_holder = '\0';
}

#ifdef __cplusplus
}
#endif

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-02-03, Man Hung-Coeng:
 *  01. Create.
 *
 * >>> 2022-02-18, Man Hung-Coeng:
 *  01. Change some parameter and return value types from int*_t to uint*_t.
 *  02. Add some macros to make defining and using struct meta variables easier.
 *
 * >>> 2022-03-08, Man Hung-Coeng:
 *  01. Fix the error of checking whether meta_ptr is out of range
 *      in calculate_struct_size(), and rename this function at the same time.
 */

