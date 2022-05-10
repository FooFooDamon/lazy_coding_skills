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

enum
{
    MB = 1024 * 1024
};

#define COMMPROTO_EXPAND_BUFSIZE(old_size)  (((old_size) < MB) ? ((old_size) * 2) : (MB * ((old_size) / MB + 1)))

#ifndef COMMPROTO_MAX_BUFSIZE
#define COMMPROTO_MAX_BUFSIZE               (1024 * 1024 * 4)
#endif

#if 0 != COMMPROTO_MAX_BUFSIZE % 1024
#error COMMPROTO_MAX_BUFSIZE is not multiples of 1024!
#endif

#if COMMPROTO_MAX_BUFSIZE < 1024
#error COMMPROTO_MAX_BUFSIZE must be greater than or equal to 1024!
#endif

#if COMMPROTO_MAX_BUFSIZE > 1024 * 1024 * 64
#error COMMPROTO_MAX_BUFSIZE must be less than or equal to 1024 * 1024 * 64!
#endif

#ifndef COMMPROTO_INITIAL_BUFSIZE
#define COMMPROTO_INITIAL_BUFSIZE           128
#endif

#if (COMMPROTO_INITIAL_BUFSIZE != 4) && (COMMPROTO_INITIAL_BUFSIZE != 8) && \
    (COMMPROTO_INITIAL_BUFSIZE != 16) && (COMMPROTO_INITIAL_BUFSIZE != 32) && \
    (COMMPROTO_INITIAL_BUFSIZE != 64) && (COMMPROTO_INITIAL_BUFSIZE != 128)
#error COMMPROTO_INITIAL_BUFSIZE must be 4, 8, 16, 32, 64, 128!
#endif

static int32_t calc_struct_size_or_move_meta_ptr(int16_t struct_field_count, const uint8_t *meta_ptr,
    uint32_t meta_len, uint8_t **nullable_meta_pptr)
{
    const uint8_t *meta_start_ptr = meta_ptr;
    int32_t result = 0;
    int16_t i = 0;

    for (; i < struct_field_count; ++i)
    {
        uint8_t type = *meta_ptr;

        if (type < COMMPROTO_SIMPLE_FIELD_TYPE_END)
        {
            bool is_single_type = (type < COMMPROTO_SINGLE_FIELD_TYPE_END);
            bool is_static_array = (type >= COMMPROTO_INT8_FIXED_ARRAY);

            if (is_single_type || is_static_array)
                result += ((type % 10) * (is_static_array ? *((int16_t *)(meta_ptr + sizeof(int8_t))) : 1));
            else
                result += sizeof(ptrdiff_t);

            meta_ptr += (sizeof(int8_t) + (is_static_array ? sizeof(int16_t) : 0));
        }
        else if (type == COMMPROTO_STRUCT_DYNAMIC_ARRAY)
        {
            uint8_t *sub_meta_ptr = (uint8_t *)meta_ptr + sizeof(int8_t) + sizeof(int16_t);
            int32_t sub_struct_size = calc_struct_size_or_move_meta_ptr(
                *((int16_t *)(meta_ptr + sizeof(int8_t))), sub_meta_ptr,
                meta_len - sizeof(int8_t) - sizeof(int16_t), &sub_meta_ptr
            );

            if (sub_struct_size < 0)
            {
                if (NULL != nullable_meta_pptr)
                    *nullable_meta_pptr = sub_meta_ptr;

                return sub_struct_size;
            }

            result += sizeof(ptrdiff_t);

            meta_ptr = sub_meta_ptr;
        }
        else if (type == COMMPROTO_STRUCT_FIXED_ARRAY)
        {
            uint8_t *sub_meta_ptr = (uint8_t *)meta_ptr + sizeof(int8_t) + sizeof(int16_t) * 2;
            int16_t sub_struct_count = *((int16_t *)(meta_ptr + sizeof(int8_t) + sizeof(int16_t)));
            int32_t sub_struct_size = calc_struct_size_or_move_meta_ptr(
                *((int16_t *)(meta_ptr + sizeof(int8_t))), sub_meta_ptr,
                meta_len - sizeof(int8_t) - sizeof(int16_t) * 2, &sub_meta_ptr
            );

            if (sub_struct_size < 0)
            {
                if (NULL != nullable_meta_pptr)
                    *nullable_meta_pptr = sub_meta_ptr;

                return sub_struct_size;
            }

            result += (sub_struct_size * sub_struct_count);

            meta_ptr = sub_meta_ptr;
        }
        else
        {
            result = -COMMPROTO_ERR_WRONG_META_DATA;
            break;
        }

        if (meta_ptr - meta_start_ptr >= meta_len)
            break;
    } /* for (; i < struct_field_count; ++i) */

    if (NULL != nullable_meta_pptr)
        *nullable_meta_pptr = (uint8_t *)meta_ptr;

    COMMPROTO_DPRINT("struct size: %d\n", result);

    return result;
}

static int general_serialization(int16_t fields, int16_t loops, bool can_have_inner_struct,
    const uint8_t *meta_start_ptr, uint32_t meta_len, uint8_t **meta_pptr,
    uint8_t **struct_pptr, bool is_static_buf, uint32_t max_buf_len,
    uint8_t **buf_pptr, uint32_t *buf_capacity_ptr, uint32_t *handled_len_ptr)
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
            uint8_t type = **meta_pptr;
            bool is_dynamic_struct_array = (COMMPROTO_STRUCT_DYNAMIC_ARRAY == type);
            uint32_t meta_offset = sizeof(int8_t);
            int32_t data_offset = 0;

            switch (type) /* Step 1: Determine offsets and array lengths. */
            {
            case COMMPROTO_INT8:
            case COMMPROTO_INT16:
            case COMMPROTO_INT32:
            case COMMPROTO_INT64:
            case COMMPROTO_FLOAT32:
            case COMMPROTO_FLOAT64:
            case COMMPROTO_ARRAY_LEN:
                if (COMMPROTO_ARRAY_LEN == type)
                {
                    simple_array_len = struct_array_len = *((int16_t *)*struct_pptr);
                }
                data_offset += (type % 10);
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

            default:
                err = -COMMPROTO_ERR_UNKNOWN_FIELD_TYPE;
                continue;
            } /* switch (type) */

            *meta_pptr += meta_offset;

            if (*handled_len_ptr + data_offset > *buf_capacity_ptr) /* Step 2: Expand the buffer if needed. */
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

            if (type < COMMPROTO_SIMPLE_FIELD_TYPE_END)
                *handled_len_ptr += data_offset;

            #define CASE_FOR_SIMPLE_FIELD_SERIALIZATION(T, W)       \
                case COMMPROTO_##T##W: \
                case COMMPROTO_##T##W##_DYNAMIC_ARRAY: \
                case COMMPROTO_##T##W##_FIXED_ARRAY: \
                    COMMPROTO_SET_##T##W##_ARRAY(((COMMPROTO_##T##W == type) ? 1 : simple_array_len), \
                        ((COMMPROTO_##T##W##_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr), \
                        *buf_pptr + *handled_len_ptr - data_offset); \
                    *struct_pptr += ((COMMPROTO_##T##W##_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset); \
                    break

            switch (type) /* Step 3: Do assignments. */
            {
            CASE_FOR_SIMPLE_FIELD_SERIALIZATION(INT, 8);

            CASE_FOR_SIMPLE_FIELD_SERIALIZATION(INT, 16);

            CASE_FOR_SIMPLE_FIELD_SERIALIZATION(INT, 32);

            CASE_FOR_SIMPLE_FIELD_SERIALIZATION(INT, 64);

            CASE_FOR_SIMPLE_FIELD_SERIALIZATION(FLOAT, 32);

            CASE_FOR_SIMPLE_FIELD_SERIALIZATION(FLOAT, 64);

            case COMMPROTO_ARRAY_LEN:
                COMMPROTO_SET_INT16(*struct_pptr, *buf_pptr + *handled_len_ptr - data_offset);
                *struct_pptr += data_offset;
                break;

            default:
                inner_struct_ptr = (is_dynamic_struct_array ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : NULL);
                err = (/*0 == struct_field_count || */0 == struct_array_len) ? 0
                    : general_serialization(struct_field_count, struct_array_len, /* can_have_inner_struct = */false,
                        meta_start_ptr, meta_len, meta_pptr, (is_dynamic_struct_array ? &inner_struct_ptr : struct_pptr),
                        is_static_buf, max_buf_len, buf_pptr, buf_capacity_ptr, handled_len_ptr);
                if (is_dynamic_struct_array && err >= 0)
                {
                    *struct_pptr += sizeof(ptrdiff_t);
                    if (0 == struct_array_len/* && struct_field_count > 0*/)
                        calc_struct_size_or_move_meta_ptr(struct_field_count, *meta_pptr, meta_len, meta_pptr);
                }
                break;
            } /* switch (type) */

            COMMPROTO_DPRINT("loop[%d/%d] Serialization: type: %d, handled len: %d, meta offset: %d,"
                " struct offset: %d\n", loop, loops, type, (int)*handled_len_ptr, (int)(*meta_pptr - meta_start_ptr),
                (int)(*struct_pptr - struct_ptr_start));

            if (*meta_pptr - meta_start_ptr >= meta_len || ++field > fields)
                break;
        } /* while (err >= 0) */

        if (0 == err && loop < loops)
            *meta_pptr = meta_ptr_per_round;
    } /* for (; loop <= loops && err >= 0; ++loop) */

    return err;
}

commproto_result_t commproto_serialize(const uint8_t *struct_meta_data, uint32_t meta_len,
    const void *one_byte_aligned_struct, uint8_t *nullable_buf, uint32_t buf_len)
{
    uint8_t *meta_ptr = (uint8_t *)struct_meta_data;
    uint8_t *struct_ptr = (uint8_t *)one_byte_aligned_struct;
    bool is_static_buf = (NULL != nullable_buf);
    const uint32_t MAX_BUF_LEN = is_static_buf ? buf_len : COMMPROTO_MAX_BUFSIZE;
    uint32_t buf_capacity = is_static_buf ? buf_len : COMMPROTO_INITIAL_BUFSIZE;
    uint8_t *new_buf = NULL;
    commproto_result_t result = { 0 };

    result.buf_ptr = is_static_buf ? nullable_buf : (uint8_t *)malloc(buf_capacity);
    result.buf_len = buf_capacity;
    result.error_code = s_is_initialized
        ? (
            is_static_buf
            ? ((0 == result.buf_len) ? -COMMPROTO_ERR_ZERO_LENGTH : 0)
            : ((NULL == result.buf_ptr) ? -COMMPROTO_ERR_MEM_ALLOC : 0)
        )
        : -COMMPROTO_ERR_NOT_INITIALIZED;

    if (result.error_code < 0)
        goto SERIALIZE_END;

    result.error_code = general_serialization(
        /* fields = */0xffff / 2, /* loops = */1, /* can_have_inner_struct = */true,
        struct_meta_data, meta_len, &meta_ptr,
        &struct_ptr, is_static_buf, MAX_BUF_LEN,
        &result.buf_ptr, &result.buf_len, &result.handled_len
    );
    if (result.error_code < 0)
    {
        goto SERIALIZE_END;
    }

    if ((!is_static_buf) && result.handled_len < result.buf_len
        && NULL != (new_buf = (uint8_t *)realloc(result.buf_ptr, result.handled_len)))
    {
        result.buf_ptr = new_buf;
        result.buf_len = result.handled_len;
    }

SERIALIZE_END:

    return result;
}

static int general_deserialization(int16_t fields, int16_t loops, bool can_have_inner_struct,
    const uint8_t *meta_start_ptr, uint32_t meta_len, uint8_t **meta_pptr,
    const uint8_t *buf_ptr, uint32_t buf_len,
    uint8_t **struct_pptr, int32_t struct_size,
    uint32_t *handled_len_ptr)
{
    int16_t simple_array_len = -1;
    int16_t struct_array_len = -1;
    int16_t struct_field_count = -1;
    uint8_t *inner_struct_ptr = NULL;
    bool should_reallocate_array = false;
    uint8_t *meta_ptr_per_round = *meta_pptr;
    uint8_t *struct_ptr_start = *struct_pptr;
    int err = 0;
    int16_t loop = 1;

    for (; loop <= loops && err >= 0; ++loop)
    {
        int16_t field = 1;

        while (err >= 0 && *handled_len_ptr < buf_len)
        {
            uint8_t type = **meta_pptr;
            bool is_dynamic_struct_array = (COMMPROTO_STRUCT_DYNAMIC_ARRAY == type);
            int32_t dynamic_array_size = 0;
            int32_t static_struct_array_size = 0;
            uint32_t meta_offset = sizeof(int8_t);
            int32_t data_offset = 0;

            switch (type) /* Step 1: Determine offsets and array lengths. */
            {
            case COMMPROTO_INT8:
            case COMMPROTO_INT16:
            case COMMPROTO_INT32:
            case COMMPROTO_INT64:
            case COMMPROTO_FLOAT32:
            case COMMPROTO_FLOAT64:
            case COMMPROTO_ARRAY_LEN:
                data_offset += (type % 10);
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

            case COMMPROTO_STRUCT_FIXED_ARRAY:
                if (!can_have_inner_struct)
                {
                    err = -COMMPROTO_ERR_WRONG_META_DATA;
                    continue;
                }
                struct_field_count = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                struct_array_len = *((int16_t *)(*meta_pptr + sizeof(int8_t) + sizeof(int16_t)));
                meta_offset += (sizeof(int16_t) * 2);
                static_struct_array_size = calc_struct_size_or_move_meta_ptr(struct_field_count,
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

            /*
             * Step 2: Allocate or reallocate memory for current dynamic array if needed.
             *
             * NOTE: The memory can be allocated only once and reused repeatedly if and only if:
             *      1) the same struct instance is used repeatedly,
             *      2) maximum lengths of the instance's dynamic sub-arrays are fixed,
             *      3) lengths of the instance's sub-arrays are set to their maxinum values on first use,
             *          and will not be greater afterwards.
             *      However, if the rules above are broken, memory leaks may occur!
             *      If you're not sure, call commproto_clear() or release the memory manually
             *      before each new deserialization.
             */
            if (dynamic_array_size > 0 && (should_reallocate_array || NULL == (char *)**(ptrdiff_t **)struct_pptr))
            {
                char *new_array = (char *)realloc((char *)**(ptrdiff_t **)struct_pptr, dynamic_array_size);

                if (NULL == new_array)
                {
                    err = -COMMPROTO_ERR_MEM_ALLOC;
                    continue;
                }
                COMMPROTO_DPRINT("(Re)allocated the array: addr = %p, size = %d\n", new_array, dynamic_array_size);

                if (can_have_inner_struct && new_array != (char *)**(ptrdiff_t **)struct_pptr)
                    memset(new_array, 0, dynamic_array_size);

                **(ptrdiff_t **)struct_pptr = (ptrdiff_t)new_array;
            }

            if (type < COMMPROTO_SIMPLE_FIELD_TYPE_END)
                *handled_len_ptr += data_offset;

            #define CASE_FOR_SIMPLE_FIELD_PARSING(T, W)             \
                case COMMPROTO_##T##W: \
                case COMMPROTO_##T##W##_DYNAMIC_ARRAY: \
                case COMMPROTO_##T##W##_FIXED_ARRAY: \
                    COMMPROTO_SET_##T##W##_ARRAY(((COMMPROTO_##T##W == type) ? 1 : simple_array_len), \
                        buf_ptr + *handled_len_ptr - data_offset, \
                        ((COMMPROTO_##T##W##_DYNAMIC_ARRAY == type) ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : *struct_pptr)); \
                    *struct_pptr += ((COMMPROTO_##T##W##_DYNAMIC_ARRAY == type) ? sizeof(ptrdiff_t) : data_offset); \
                    break

            switch (type) /* Step 3: Do assignments. */
            {
            CASE_FOR_SIMPLE_FIELD_PARSING(INT, 8);

            CASE_FOR_SIMPLE_FIELD_PARSING(INT, 16);

            CASE_FOR_SIMPLE_FIELD_PARSING(INT, 32);

            CASE_FOR_SIMPLE_FIELD_PARSING(INT, 64);

            CASE_FOR_SIMPLE_FIELD_PARSING(FLOAT, 32);

            CASE_FOR_SIMPLE_FIELD_PARSING(FLOAT, 64);

            case COMMPROTO_ARRAY_LEN:
                COMMPROTO_SET_INT16(buf_ptr + *handled_len_ptr - data_offset, &simple_array_len);
                should_reallocate_array = (simple_array_len > *((int16_t *)*struct_pptr));
                *((int16_t *)*struct_pptr) = struct_array_len = simple_array_len;
                *struct_pptr += data_offset;
                break;

            default:
                inner_struct_ptr = (is_dynamic_struct_array ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : NULL);
                err = (0 == struct_array_len) ? 0
                    : general_deserialization(struct_field_count, struct_array_len, /* can_have_inner_struct = */false,
                        meta_start_ptr, meta_len, meta_pptr, buf_ptr, buf_len,
                        (is_dynamic_struct_array ? &inner_struct_ptr : struct_pptr),
                        (is_dynamic_struct_array ? dynamic_array_size : static_struct_array_size),
                        handled_len_ptr);
                if (is_dynamic_struct_array && err >= 0)
                {
                    *struct_pptr += sizeof(ptrdiff_t);
                    if (0 == struct_array_len)
                        calc_struct_size_or_move_meta_ptr(struct_field_count, *meta_pptr, meta_len, meta_pptr);
                }
                break;
            } /* switch (type) */

            COMMPROTO_DPRINT("loop[%d/%d] Deserialization: type: %d, handled len: %d, meta offset: %d,"
                " struct offset: %d\n", loop, loops, type, (int)*handled_len_ptr, (int)(*meta_pptr - meta_start_ptr),
                (int)(*struct_pptr - struct_ptr_start));

            if (*struct_pptr - struct_ptr_start > struct_size)
            {
                COMMPROTO_DPRINT("*** struct pointer exceeds maxinum offset: %d\n", struct_size);
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

commproto_result_t commproto_parse(const uint8_t *struct_meta_data, uint32_t meta_len,
    const uint8_t *buf_ptr, uint32_t buf_len, void *one_byte_aligned_struct)
{
    uint8_t *meta_ptr = (uint8_t *)struct_meta_data;
    uint8_t *struct_ptr = (uint8_t *)one_byte_aligned_struct;
    const int32_t STRUCT_SIZE = calc_struct_size_or_move_meta_ptr(0xffff / 2, meta_ptr, meta_len, NULL);
    commproto_result_t result = { 0 };

    result.buf_ptr = (uint8_t *)buf_ptr;
    result.buf_len = buf_len;
    result.error_code = s_is_initialized
        ? (
            (0 == result.buf_len)
            ? -COMMPROTO_ERR_ZERO_LENGTH
            : general_deserialization(
                /* fields = */0xffff / 2, /* loops = */1, /* can_have_inner_struct = */true,
                struct_meta_data, meta_len, &meta_ptr, result.buf_ptr, result.buf_len,
                &struct_ptr, STRUCT_SIZE, &result.handled_len
            )
        )
        : -COMMPROTO_ERR_NOT_INITIALIZED;

    return result;
}

static int general_clear(int16_t fields, int16_t loops, bool can_have_inner_struct,
    const uint8_t *meta_start_ptr, uint32_t meta_len, uint8_t **meta_pptr, uint8_t **struct_pptr)
{
    int16_t simple_array_len = -1;
    int16_t struct_array_len = -1;
    int16_t struct_field_count = -1;
    uint8_t *inner_struct_ptr = NULL;
    uint8_t *meta_ptr_per_round = *meta_pptr;
#if defined(COMMPROTO_DEBUG) || defined(TEST)
    uint8_t *struct_ptr_start = *struct_pptr;
#endif
    int err = 0;
    int16_t loop = 1;

    for (; loop <= loops && err >= 0; ++loop)
    {
        int16_t field = 1;

        while (err >= 0)
        {
            uint8_t type = **meta_pptr;

            switch (type)
            {
            case COMMPROTO_INT8:
            case COMMPROTO_INT16:
            case COMMPROTO_INT32:
            case COMMPROTO_INT64:
            case COMMPROTO_FLOAT32:
            case COMMPROTO_FLOAT64:
            case COMMPROTO_ARRAY_LEN:
                if (COMMPROTO_ARRAY_LEN == type)
                {
                    simple_array_len = struct_array_len = *((int16_t *)*struct_pptr);
                }
                *struct_pptr += (type % 10);
                *meta_pptr += sizeof(int8_t);
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
                free((uint8_t *)**(ptrdiff_t **)struct_pptr);
                COMMPROTO_DPRINT("Freed a simple array: %p\n", (uint8_t *)**(ptrdiff_t **)struct_pptr);
                **(ptrdiff_t **)struct_pptr = (ptrdiff_t)NULL;
                *struct_pptr += sizeof(ptrdiff_t);
                *meta_pptr += sizeof(int8_t);
                break;

            case COMMPROTO_INT8_FIXED_ARRAY:
            case COMMPROTO_INT16_FIXED_ARRAY:
            case COMMPROTO_INT32_FIXED_ARRAY:
            case COMMPROTO_INT64_FIXED_ARRAY:
            case COMMPROTO_FLOAT32_FIXED_ARRAY:
            case COMMPROTO_FLOAT64_FIXED_ARRAY:
                simple_array_len = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                *struct_pptr += ((type % 10) * simple_array_len);
                *meta_pptr += (sizeof(int8_t) + sizeof(int16_t));
                break;

            case COMMPROTO_STRUCT_DYNAMIC_ARRAY:
                if (struct_array_len < 0 || !can_have_inner_struct)
                {
                    err = (struct_array_len < 0) ? -COMMPROTO_ERR_META_ARRAY_LENGTH_MISSING
                        : -COMMPROTO_ERR_WRONG_META_DATA;
                    continue;
                }
                struct_field_count = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                *meta_pptr += (sizeof(int8_t) + sizeof(int16_t));
                break;

            case COMMPROTO_STRUCT_FIXED_ARRAY:
                if (!can_have_inner_struct)
                {
                    err = -COMMPROTO_ERR_WRONG_META_DATA;
                    continue;
                }
                struct_field_count = *((int16_t *)(*meta_pptr + sizeof(int8_t)));
                struct_array_len = *((int16_t *)(*meta_pptr + sizeof(int8_t) + sizeof(int16_t)));
                *meta_pptr += (sizeof(int8_t) + sizeof(int16_t) * 2);
                break;

            default:
                err = -COMMPROTO_ERR_UNKNOWN_FIELD_TYPE;
                continue;
            } /* switch (type) */

            if (type > COMMPROTO_SIMPLE_FIELD_TYPE_END)
            {
                bool is_dynamic_struct_array = (COMMPROTO_STRUCT_DYNAMIC_ARRAY == type);

                inner_struct_ptr = (is_dynamic_struct_array ? ((uint8_t *)**(ptrdiff_t **)struct_pptr) : NULL);
                err = (0 == struct_array_len || (is_dynamic_struct_array && NULL == inner_struct_ptr)) ? 0
                    : general_clear(/* fields = */struct_field_count, /* loops = */struct_array_len,
                        /* can_have_inner_struct = */false, meta_start_ptr, meta_len, meta_pptr,
                        (is_dynamic_struct_array ? &inner_struct_ptr : struct_pptr));
                if (is_dynamic_struct_array && err >= 0)
                {
                    free((uint8_t *)**(ptrdiff_t **)struct_pptr); /* NOTE: DO NOT: free(inner_struct_ptr); */
                    COMMPROTO_DPRINT("Freed a struct array: %p\n", (uint8_t *)**(ptrdiff_t **)struct_pptr);
                    **(ptrdiff_t **)struct_pptr = (ptrdiff_t)NULL;
                    *struct_pptr += sizeof(ptrdiff_t);
                    if (0 == struct_array_len)
                        calc_struct_size_or_move_meta_ptr(struct_field_count, *meta_pptr, meta_len, meta_pptr);
                }
            }

            COMMPROTO_DPRINT("loop[%d/%d] Clear: type: %d, meta offset: %d, struct offset: %d\n",
                loop, loops, type, (int)(*meta_pptr - meta_start_ptr), (int)(*struct_pptr - struct_ptr_start));

            if (*meta_pptr - meta_start_ptr >= meta_len || ++field > fields)
                break;
        } /* while (err >= 0) */

        if (0 == err && loop < loops)
            *meta_pptr = meta_ptr_per_round;
    } /* for (; loop <= loops && err >= 0; ++loop) */

    return err;
}

void commproto_clear(const uint8_t *struct_meta_data, uint32_t meta_len, void *one_byte_aligned_struct)
{
    if (s_is_initialized)
    {
        uint8_t *meta_ptr = (uint8_t *)struct_meta_data;
        uint8_t *struct_ptr = (uint8_t *)one_byte_aligned_struct;

        general_clear(/* fields = */0xffff / 2, /* loops = */1, /* can_have_inner_struct = */true,
            struct_meta_data, meta_len, &meta_ptr, &struct_ptr);
    }
}

void commproto_dump_buffer(const uint8_t *buf, uint32_t size, FILE *nullable_stream, char *nullable_holder)
{
    char hex1[3 * 8 + 1] = { 0 };
    char hex2[3 * 8 + 1] = { 0 };
    char str1[8 + 1] = { 0 };
    char str2[8 + 1] = { 0 };
    uint32_t i = 0;

    for (; i < size; ++i)
    {
        uint8_t remainder = i % 16;

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
            fprintf(nullable_stream, "%05d", (int)(i / 16));
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
            sprintf(nullable_holder, "%05d", (int)(i / 16));
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
 *
 * >>> 2022-05-06, Man Hung-Coeng:
 *  01. Add function commproto_clear() for struct memory release.
 *
 * >>> 2022-05-07, Man Hung-Coeng:
 *  01. Change the type of return value of commproto_serialize() and
 *      commproto_parse() to commproto_result_t (a new added type)
 *      in order to reduce the amount of function parameters.
 *
 * >>> 2022-05-08, Man Hung-Coeng:
 *  01. Fix the potential struct-pointer-exceeds problem.
 *  02. Change types of *_len parameters/fields from [u]int16_t to [u]int32_t.
 *  03. Refactor.
 *
 * >>> 2022-05-10, Man Hung-Coeng:
 *  01. Fix the conditional-jump-or-move-depends-on-uninitialised-values problem
 *      reported by valgrind.
 */

