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

#ifndef __COMMUNICATION_PROTOCOL_H__
#define __COMMUNICATION_PROTOCOL_H__

#include <stdint.h> /* For int8_t, int16_t, etc. */
#include <string.h> /* For memcpy(). */
#include <stdio.h> /* For FILE type. */

#ifdef __cplusplus

#if __cplusplus >= 201103L /* C++11 or above. */
#define COMMPROTO_CPP_CONSTEXPR                         constexpr
#else
#define COMMPROTO_CPP_CONSTEXPR                         const
#endif

#define COMMPROTO_DEFINE_META_FUNCTIONS_IN_STRUCT()     \
    static inline const uint8_t* meta_data(void) \
    { \
        return (uint8_t *)__META_DATA__; \
    } \
\
    static inline COMMPROTO_CPP_CONSTEXPR uint32_t meta_size(void) \
    { \
        return sizeof(__META_DATA__); \
    }

#define COMMPROTO_META_VAR_IN_STRUCT                    static COMMPROTO_CPP_CONSTEXPR uint8_t __META_DATA__[]

#define COMMPROTO_META_VAR_OUT_OF_STRUCT(struct_name)   COMMPROTO_CPP_CONSTEXPR uint8_t struct_name::__META_DATA__[]

#define COMMPROTO_DEFINE_DESTRUCTOR(struct_name)        ~struct_name() { \
    commproto_clear(struct_name::meta_data(), struct_name::meta_size(), this); \
}

/* NOTE: These macros are only suitable for structs WITHOUT virtual functions! */

#define COMMPROTO_CPP_SERIALIZE(struct_ptr, buf_ptr, buf_len)   \
    commproto_serialize((struct_ptr)->meta_data(), (struct_ptr)->meta_size(), struct_ptr, buf_ptr, buf_len)

#define COMMPROTO_CPP_PARSE(buf_ptr, buf_len, struct_ptr)       \
    commproto_parse((struct_ptr)->meta_data(), (struct_ptr)->meta_size(), buf_ptr, buf_len, struct_ptr)

#define COMMPROTO_CPP_CLEAR(struct_ptr)                         \
    commproto_clear((struct_ptr)->meta_data(), (struct_ptr)->meta_size(), struct_ptr)

/* NOTE: These macros are only suitable for structs CONTAINING virtual functions! */

#define COMMPROTO_CPP_VSERIALIZE(struct_ptr, buf_ptr, buf_len)  \
    commproto_serialize((struct_ptr)->meta_data(), (struct_ptr)->meta_size(), (char*)(struct_ptr) + sizeof(void*), buf_ptr, buf_len)

#define COMMPROTO_CPP_VPARSE(buf_ptr, buf_len, struct_ptr)      \
    commproto_parse((struct_ptr)->meta_data(), (struct_ptr)->meta_size(), buf_ptr, buf_len, (char*)(struct_ptr) + sizeof(void*))

#define COMMPROTO_CPP_VCLEAR(struct_ptr)                        \
    commproto_clear((struct_ptr)->meta_data(), (struct_ptr)->meta_size(), (char*)(struct_ptr) + sizeof(void*))

extern "C" {

#endif /* #ifdef __cplusplus */

typedef uint16_t    arraylen_t;
typedef float       float32_t;  /* TODO: To be more portable. */
typedef double      float64_t;  /* TODO: Same as above. */

typedef struct commproto_result_t
{
    uint8_t *buf_ptr;
    uint32_t buf_len;
    uint32_t handled_len;
    int error_code;
} commproto_result_t;

const char* commproto_error(int error_code);

int commproto_init(void);

commproto_result_t commproto_serialize(const uint8_t *struct_meta_data, uint32_t meta_len,
    const void *one_byte_aligned_struct, uint8_t *nullable_buf, uint32_t buf_len);

commproto_result_t commproto_parse(const uint8_t *struct_meta_data, uint32_t meta_len,
    const uint8_t *buf_ptr, uint32_t buf_len, void *one_byte_aligned_struct);

void commproto_clear(const uint8_t *struct_meta_data, uint32_t meta_len, void *one_byte_aligned_struct);

void commproto_dump_buffer(const uint8_t *buf, uint32_t size, FILE *nullable_stream, char *nullable_holder);

#define COMMPROTO_META_VAR(struct_name)                 META_DATA_##struct_name
#define COMMPROTO_DECLARE_META_VAR(struct_name)         const uint8_t META_DATA_##struct_name[]

#define COMMPROTO_META_SIZE(struct_name)                META_SIZE_##struct_name
#define COMMPROTO_DECLARE_META_SIZE(struct_name)        const uint32_t META_SIZE_##struct_name
#define COMMPROTO_DEFINE_META_SIZE(struct_name)         const uint32_t META_SIZE_##struct_name = sizeof(META_DATA_##struct_name)

#define COMMPROTO_SERIALIZE(struct_name, struct_ptr, buf_ptr, buf_len)                      \
    commproto_serialize(COMMPROTO_META_VAR(struct_name), COMMPROTO_META_SIZE(struct_name), struct_ptr, buf_ptr, buf_len)

#define COMMPROTO_PARSE(struct_name, buf_ptr, buf_len, struct_ptr)                          \
    commproto_parse(COMMPROTO_META_VAR(struct_name), COMMPROTO_META_SIZE(struct_name), buf_ptr, buf_len, struct_ptr)

#define COMMPROTO_CLEAR(struct_name, struct_ptr)                                            \
    commproto_clear(COMMPROTO_META_VAR(struct_name), COMMPROTO_META_SIZE(struct_name), struct_ptr)

enum
{
    COMMPROTO_INT8 = 1
    , COMMPROTO_INT16 = 2
    , COMMPROTO_INT32 = 4
    , COMMPROTO_INT64 = 8
    , COMMPROTO_FLOAT32 = 14
    , COMMPROTO_FLOAT64 = 18

    , COMMPROTO_ARRAY_LEN = 20 + sizeof(arraylen_t)

    , COMMPROTO_SINGLE_FIELD_TYPE_END /* Generally for inner use only. */

    , COMMPROTO_INT8_DYNAMIC_ARRAY = 31
    , COMMPROTO_INT16_DYNAMIC_ARRAY = 32
    , COMMPROTO_INT32_DYNAMIC_ARRAY = 34
    , COMMPROTO_INT64_DYNAMIC_ARRAY = 38
    , COMMPROTO_FLOAT32_DYNAMIC_ARRAY = 44
    , COMMPROTO_FLOAT64_DYNAMIC_ARRAY = 48

    , COMMPROTO_INT8_FIXED_ARRAY = 51
    , COMMPROTO_INT16_FIXED_ARRAY = 52
    , COMMPROTO_INT32_FIXED_ARRAY = 54
    , COMMPROTO_INT64_FIXED_ARRAY = 58
    , COMMPROTO_FLOAT32_FIXED_ARRAY = 64
    , COMMPROTO_FLOAT64_FIXED_ARRAY = 68

    , COMMPROTO_SIMPLE_FIELD_TYPE_END /* Generally for inner use only. */

    , COMMPROTO_STRUCT_DYNAMIC_ARRAY = 70 + sizeof(void *)
    , COMMPROTO_STRUCT_FIXED_ARRAY

    , COMMPROTO_FIELD_TYPE_END /* Generally for inner use only. */
};

#ifndef __ORDER_LITTLE_ENDIAN__
#error Macro __ORDER_LITTLE_ENDIAN__ not defined! Example: -D__ORDER_LITTLE_ENDIAN__=1234
#endif

#ifndef __ORDER_BIG_ENDIAN__
#error Macro __ORDER_BIG_ENDIAN__ not defined! Example: -D__ORDER_BIG_ENDIAN__=4321
#endif

#if __ORDER_BIG_ENDIAN__ == __ORDER_LITTLE_ENDIAN__
#error Logic error: __ORDER_BIG_ENDIAN__ == __ORDER_LITTLE_ENDIAN__ !
#endif

#ifndef __BYTE_ORDER__
#error Macro __BYTE_ORDER__ not defined! Example: -D__BYTE_ORDER__=__ORDER_LITTLE_ENDIAN__
#endif

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ && __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
#error __BYTE_ORDER__ must be equal to __ORDER_LITTLE_ENDIAN__ or __ORDER_BIG_ENDIAN__ !
#endif

#ifndef __FLOAT_WORD_ORDER__
#define __FLOAT_WORD_ORDER__    __BYTE_ORDER__
#endif

#if __FLOAT_WORD_ORDER__ != __ORDER_LITTLE_ENDIAN__ && __FLOAT_WORD_ORDER__ != __ORDER_BIG_ENDIAN__
#error __FLOAT_WORD_ORDER__ must be equal to __ORDER_LITTLE_ENDIAN__ or __ORDER_BIG_ENDIAN__ !
#endif

#if (!defined(COMMPROTO_LITTLE_ENDIAN) && !defined(COMMPROTO_BIG_ENDIAN)) \
    || (defined(COMMPROTO_LITTLE_ENDIAN) && defined(COMMPROTO_BIG_ENDIAN))
#error One and only one of COMMPROTO_LITTLE_ENDIAN and COMMPROTO_BIG_ENDIAN should be defined!
#endif

#if 0 /* May cause alignment fault on some platforms. */
#define COMMPROTO_SAME_ENDIAN_ASSIGN(type, src_ptr, dest_ptr)       *((type *)(dest_ptr)) = *((type *)(src_ptr))
#else
#define COMMPROTO_SAME_ENDIAN_ASSIGN(type, src_ptr, dest_ptr)       memcpy((dest_ptr), (src_ptr), sizeof(type))
#endif

#define COMMPROTO_SAME_ENDIAN_SET(type, count, src_ptr, dest_ptr)   memcpy((dest_ptr), (src_ptr), sizeof(type) * (count))

#define COMMPROTO_DIFF_ENDIAN_ASSIGN(type, src_ptr, dest_ptr)       do { \
    uint16_t _i_ = 0; \
    for (; _i_ < sizeof(type); ++_i_) { \
        *(((uint8_t *)(dest_ptr)) + _i_) = *(((uint8_t *)(src_ptr)) + (sizeof(type) - _i_ - 1)); \
    } \
} while(0)

#define COMMPROTO_DIFF_ENDIAN_SET(type, count, src_ptr, dest_ptr)   do { \
    uint16_t _ii_ = 0, _count_ = (count); \
    for (; _ii_ < _count_; ++_ii_) { \
        uint8_t *_src_ptr_ = ((uint8_t *)(src_ptr)) + sizeof(type) * _ii_; \
        uint8_t *_dest_ptr_ = ((uint8_t *)(dest_ptr)) + sizeof(type) * _ii_; \
        COMMPROTO_DIFF_ENDIAN_ASSIGN(type, _src_ptr_, _dest_ptr_); \
    } \
} while (0)

#define COMMPROTO_SET_INT8(src_ptr, dest_ptr)                   *((uint8_t *)(dest_ptr)) = *((uint8_t *)(src_ptr))

#define COMMPROTO_SET_INT8_ARRAY(count, src_ptr, dest_ptr)      COMMPROTO_SAME_ENDIAN_SET(uint8_t, count, src_ptr, dest_ptr)

#if ((__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && defined(COMMPROTO_LITTLE_ENDIAN)) \
    || ((__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) && defined(COMMPROTO_BIG_ENDIAN))

#define COMMPROTO_SET_INT16(src_ptr, dest_ptr)                  do { \
    *((uint8_t *)(dest_ptr)) = *((uint8_t *)(src_ptr)); \
    *(((uint8_t *)(dest_ptr)) + 1) = *(((uint8_t *)(src_ptr)) + 1); \
} while (0)

#define COMMPROTO_SET_INT16_ARRAY(count, src_ptr, dest_ptr)     COMMPROTO_SAME_ENDIAN_SET(uint16_t, count, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT32(src_ptr, dest_ptr)                  COMMPROTO_SAME_ENDIAN_ASSIGN(uint32_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT32_ARRAY(count, src_ptr, dest_ptr)     COMMPROTO_SAME_ENDIAN_SET(uint32_t, count, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT64(src_ptr, dest_ptr)                  COMMPROTO_SAME_ENDIAN_ASSIGN(uint64_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT64_ARRAY(count, src_ptr, dest_ptr)     COMMPROTO_SAME_ENDIAN_SET(uint64_t, count, src_ptr, dest_ptr)

#else

#define COMMPROTO_SET_INT16(src_ptr, dest_ptr)                  do { \
    *((uint8_t *)(dest_ptr)) = *(((uint8_t *)(src_ptr)) + 1); \
    *(((uint8_t *)(dest_ptr)) + 1) = *((uint8_t *)(src_ptr)); \
} while (0)

#define COMMPROTO_SET_INT16_ARRAY(count, src_ptr, dest_ptr)     COMMPROTO_DIFF_ENDIAN_SET(uint16_t, count, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT32(src_ptr, dest_ptr)                  COMMPROTO_DIFF_ENDIAN_ASSIGN(uint32_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT32_ARRAY(count, src_ptr, dest_ptr)     COMMPROTO_DIFF_ENDIAN_SET(uint32_t, count, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT64(src_ptr, dest_ptr)                  COMMPROTO_DIFF_ENDIAN_ASSIGN(uint64_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_INT64_ARRAY(count, src_ptr, dest_ptr)     COMMPROTO_DIFF_ENDIAN_SET(uint64_t, count, src_ptr, dest_ptr)

#endif

#if ((__FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__) && defined(COMMPROTO_LITTLE_ENDIAN)) \
    || ((__FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__) && defined(COMMPROTO_BIG_ENDIAN))

#define COMMPROTO_SET_FLOAT32(src_ptr, dest_ptr)                COMMPROTO_SAME_ENDIAN_ASSIGN(float32_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_FLOAT32_ARRAY(count, src_ptr, dest_ptr)   COMMPROTO_SAME_ENDIAN_SET(float32_t, count, src_ptr, dest_ptr)

#define COMMPROTO_SET_FLOAT64(src_ptr, dest_ptr)                COMMPROTO_SAME_ENDIAN_ASSIGN(float64_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_FLOAT64_ARRAY(count, src_ptr, dest_ptr)   COMMPROTO_SAME_ENDIAN_SET(float64_t, count, src_ptr, dest_ptr)

#else

#define COMMPROTO_SET_FLOAT32(src_ptr, dest_ptr)                COMMPROTO_DIFF_ENDIAN_ASSIGN(float32_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_FLOAT32_ARRAY(count, src_ptr, dest_ptr)   COMMPROTO_DIFF_ENDIAN_SET(float32_t, count, src_ptr, dest_ptr)

#define COMMPROTO_SET_FLOAT64(src_ptr, dest_ptr)                COMMPROTO_DIFF_ENDIAN_ASSIGN(float64_t, src_ptr, dest_ptr)

#define COMMPROTO_SET_FLOAT64_ARRAY(count, src_ptr, dest_ptr)   COMMPROTO_DIFF_ENDIAN_SET(float64_t, count, src_ptr, dest_ptr)

#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define COMMPROTO_STRUCT_FIELD_COUNT(positive_num)              (positive_num) & 0xff, ((positive_num) >> 8) & 0xff

#define COMMPROTO_ARRAY_LEN_IS(positive_num)                    (positive_num) & 0xff, ((positive_num) >> 8) & 0xff

#else

#define COMMPROTO_STRUCT_FIELD_COUNT(positive_num)              ((positive_num) >> 8) & 0xff, (positive_num) & 0xff

#define COMMPROTO_ARRAY_LEN_IS(positive_num)                    ((positive_num) >> 8) & 0xff, (positive_num) & 0xff

#endif

#if defined(COMMPROTO_DEBUG) || defined(TEST)
#define COMMPROTO_DPRINT(fmt, ...)                              do { \
    if (NULL != getenv("COMMPROTO_DEBUG")) \
        printf(__FILE__ ":%d: " fmt, __LINE__, ##__VA_ARGS__);  \
} while (0)
#else
#define COMMPROTO_DPRINT(fmt, ...)
#endif

#ifdef TEST

#include <stdbool.h>
#include <stdlib.h>

#pragma pack(1) /* NOTE: Structures used for communication MUST BE 1-byte aligned! */

typedef struct demo_struct_sub1_t
{/* NOTE: Only single variables and arrays of basic types are allowed within a sub-structure. */
    int8_t i8_single;
    int16_t i16_fixed_array[1];
    arraylen_t i32_dynamic_array_len; /* NOTE: The length field MUST BE right before the target dynamic array! */
    int32_t *i32_dynamic_array;
} demo_struct_sub1_t;

typedef struct demo_struct_main_t
{
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    float32_t f32;
    float64_t f64;

    int8_t i8_fixed_array[1];
    int16_t i16_fixed_array[2];
    int32_t i32_fixed_array[3];
    int64_t i64_fixed_array[4];
    float32_t f32_fixed_array[5];
    float64_t f64_fixed_array[6];

    demo_struct_sub1_t sub1_fixed_array[3];
    arraylen_t sub1_dynamic_array_len; /* NOTE: The length field MUST BE right before the target dynamic array! */
    demo_struct_sub1_t *sub1_dynamic_array;

    arraylen_t int_dynamic_array_len; /* NOTE: One length field can be shared by multiple adjacent dynamic arrays. */
    int8_t *i8_dynamic_array;
    int16_t *i16_dynamic_array;
    int32_t *i32_dynamic_array;
    int64_t *i64_dynamic_array;
    arraylen_t f32_dynamic_array_len; /* NOTE: The length field MUST BE right before the target dynamic array! */
    float32_t *f32_dynamic_array;
    arraylen_t f64_dynamic_array_len; /* NOTE: The length field MUST BE right before the target dynamic array! */
    float64_t *f64_dynamic_array;

    arraylen_t sub2_dynamic_array_len;
    struct demo_struct_sub2_t /* NOTE: Nested structure is not recommended in C. */
    {/* NOTE: Only single variables and arrays of basic types are allowed within a sub-structure. */
        int64_t i64_single;
        float32_t f32_fixed_array[2];
        arraylen_t f64_dynamic_array_len; /* NOTE: The length field MUST BE right before the target dynamic array! */
        float64_t *f64_dynamic_array;
    } *sub2_dynamic_array, sub2_fixed_array[5];
} demo_struct_main_t;

#pragma pack() /* Restore the default byte alignment. */

#define _SUB1_STRUCT_META_DATA      COMMPROTO_INT8/* i8_single */ \
    , COMMPROTO_INT16_FIXED_ARRAY/* i16_fixed_array */, COMMPROTO_ARRAY_LEN_IS(1) \
    , COMMPROTO_ARRAY_LEN/* i32_dynamic_array_len */ \
    , COMMPROTO_INT32_DYNAMIC_ARRAY/* i32_dynamic_array */

#define _SUB2_STRUCT_META_DATA      COMMPROTO_INT64/* i64_single */ \
    , COMMPROTO_FLOAT32_FIXED_ARRAY/* f32_fixed_array */, COMMPROTO_ARRAY_LEN_IS(2) \
    , COMMPROTO_ARRAY_LEN/* f64_dynamic_array_len */ \
    , COMMPROTO_FLOAT64_DYNAMIC_ARRAY/* f64_dynamic_array */

COMMPROTO_DECLARE_META_VAR(demo_struct_main_t) = {
    COMMPROTO_INT8/* i8 */
    , COMMPROTO_INT16/* i16 */
    , COMMPROTO_INT32/* i32 */
    , COMMPROTO_INT64/* i64 */
    , COMMPROTO_FLOAT32/* f32 */
    , COMMPROTO_FLOAT64/* f64 */

    , COMMPROTO_INT8_FIXED_ARRAY/* i8_fixed_array */, COMMPROTO_ARRAY_LEN_IS(1)
    , COMMPROTO_INT16_FIXED_ARRAY/* i16_fixed_array */, COMMPROTO_ARRAY_LEN_IS(2)
    , COMMPROTO_INT32_FIXED_ARRAY/* i32_fixed_array */, COMMPROTO_ARRAY_LEN_IS(3)
    , COMMPROTO_INT64_FIXED_ARRAY/* i64_fixed_array */, COMMPROTO_ARRAY_LEN_IS(4)
    , COMMPROTO_FLOAT32_FIXED_ARRAY/* f32_fixed_array */, COMMPROTO_ARRAY_LEN_IS(5)
    , COMMPROTO_FLOAT64_FIXED_ARRAY/* f64_fixed_array */, COMMPROTO_ARRAY_LEN_IS(6)

    , COMMPROTO_STRUCT_FIXED_ARRAY/* sub1_fixed_array */, COMMPROTO_STRUCT_FIELD_COUNT(4), COMMPROTO_ARRAY_LEN_IS(3)
    , _SUB1_STRUCT_META_DATA
    , COMMPROTO_ARRAY_LEN/* sub1_dynamic_array_len */
    , COMMPROTO_STRUCT_DYNAMIC_ARRAY/* sub1_dynamic_array */, COMMPROTO_STRUCT_FIELD_COUNT(4)
    , _SUB1_STRUCT_META_DATA

    , COMMPROTO_ARRAY_LEN/* int_dynamic_array_len */
    , COMMPROTO_INT8_DYNAMIC_ARRAY/* i8_dynamic_array */
    , COMMPROTO_INT16_DYNAMIC_ARRAY/* i16_dynamic_array */
    , COMMPROTO_INT32_DYNAMIC_ARRAY/* i32_dynamic_array */
    , COMMPROTO_INT64_DYNAMIC_ARRAY/* i64_dynamic_array */
    , COMMPROTO_ARRAY_LEN/* f32_dynamic_array_len */
    , COMMPROTO_FLOAT32_DYNAMIC_ARRAY/* f32_dynamic_array */
    , COMMPROTO_ARRAY_LEN/* f64_dynamic_array_len */
    , COMMPROTO_FLOAT64_DYNAMIC_ARRAY/* f64_dynamic_array */

    , COMMPROTO_ARRAY_LEN/* sub2_dynamic_array_len */
    , COMMPROTO_STRUCT_DYNAMIC_ARRAY/* sub2_dynamic_array */, COMMPROTO_STRUCT_FIELD_COUNT(4)
    , _SUB2_STRUCT_META_DATA
    , COMMPROTO_STRUCT_FIXED_ARRAY/* sub2_fixed_array */, COMMPROTO_STRUCT_FIELD_COUNT(4), COMMPROTO_ARRAY_LEN_IS(5)
    , _SUB2_STRUCT_META_DATA
};

COMMPROTO_DEFINE_META_SIZE(demo_struct_main_t);

static void fill_demo_struct(demo_struct_main_t *demo_struct)
{
    size_t i, j;

    demo_struct->i8 = 8;
    demo_struct->i16 = 16;
    demo_struct->i32 = 32;
    demo_struct->i64 = 64;
    demo_struct->f32 = 32.32;
    demo_struct->f64 = 64.64;

    for (i = 0; i < sizeof(demo_struct->i8_fixed_array) / sizeof(int8_t); ++i)
    {
        demo_struct->i8_fixed_array[i] = 8;
    }

    for (i = 0; i < sizeof(demo_struct->i16_fixed_array) / sizeof(int16_t); ++i)
    {
        demo_struct->i16_fixed_array[i] = 16;
    }

    for (i = 0; i < sizeof(demo_struct->i32_fixed_array) / sizeof(int32_t); ++i)
    {
        demo_struct->i32_fixed_array[i] = 32;
    }

    for (i = 0; i < sizeof(demo_struct->i64_fixed_array) / sizeof(int64_t); ++i)
    {
        demo_struct->i64_fixed_array[i] = 64;
    }

    for (i = 0; i < sizeof(demo_struct->f32_fixed_array) / sizeof(float32_t); ++i)
    {
        demo_struct->f32_fixed_array[i] = 32.32;
    }

    for (i = 0; i < sizeof(demo_struct->f64_fixed_array) / sizeof(float64_t); ++i)
    {
        demo_struct->f64_fixed_array[i] = 64.64;
    }

    for (i = 0; i < sizeof(demo_struct->sub1_fixed_array) / sizeof(demo_struct_sub1_t); ++i)
    {
        demo_struct_sub1_t *sub1 = &demo_struct->sub1_fixed_array[i];

        sub1->i8_single = 8;
        for (j = 0; j < sizeof(sub1->i16_fixed_array) / sizeof(int16_t); ++j)
        {
            sub1->i16_fixed_array[j] = 16;
        }
        sub1->i32_dynamic_array_len = i;
        sub1->i32_dynamic_array = malloc(sizeof(int32_t) * sub1->i32_dynamic_array_len);
        for (j = 0; j < (size_t)sub1->i32_dynamic_array_len; ++j)
        {
            sub1->i32_dynamic_array[j] = 32;
        }
    }

    demo_struct->sub1_dynamic_array_len = 4;
    demo_struct->sub1_dynamic_array = malloc(sizeof(demo_struct_sub1_t) * demo_struct->sub1_dynamic_array_len);
    for (i = 0; i < (size_t)demo_struct->sub1_dynamic_array_len; ++i)
    {
        demo_struct_sub1_t *sub1 = &demo_struct->sub1_dynamic_array[i];

        sub1->i8_single = 8;
        for (j = 0; j < sizeof(sub1->i16_fixed_array) / sizeof(int16_t); ++j)
        {
            sub1->i16_fixed_array[j] = 16;
        }
        sub1->i32_dynamic_array_len = i;
        sub1->i32_dynamic_array = malloc(sizeof(int32_t) * sub1->i32_dynamic_array_len);
        for (j = 0; j < (size_t)sub1->i32_dynamic_array_len; ++j)
        {
            sub1->i32_dynamic_array[j] = 32;
        }
    }

    demo_struct->int_dynamic_array_len = 1;
    demo_struct->i8_dynamic_array = malloc(sizeof(int8_t) * demo_struct->int_dynamic_array_len);
    demo_struct->i16_dynamic_array = malloc(sizeof(int16_t) * demo_struct->int_dynamic_array_len);
    demo_struct->i32_dynamic_array = malloc(sizeof(int32_t) * demo_struct->int_dynamic_array_len);
    demo_struct->i64_dynamic_array = malloc(sizeof(int64_t) * demo_struct->int_dynamic_array_len);
    for (i = 0; i < (size_t)demo_struct->int_dynamic_array_len; ++i)
    {
        demo_struct->i8_dynamic_array[i] = 8;
        demo_struct->i16_dynamic_array[i] = 16;
        demo_struct->i32_dynamic_array[i] = 32;
        demo_struct->i64_dynamic_array[i] = 64;
    }

    demo_struct->f32_dynamic_array_len = 2;
    demo_struct->f32_dynamic_array = malloc(sizeof(float32_t) * demo_struct->f32_dynamic_array_len);
    for (i = 0; i < (size_t)demo_struct->f32_dynamic_array_len; ++i)
    {
        demo_struct->f32_dynamic_array[i] = 32.32;
    }

    /* Keep f64_dynamic_array empty on purpose. */

    /* Keep sub2_dynamic_array empty on purpose. */

    for (i = 0; i < 5; ++i)
    {
        demo_struct->sub2_fixed_array[i].i64_single = 64;
        for (j = 0; j < sizeof(demo_struct->sub2_fixed_array[i].f32_fixed_array) / sizeof(float32_t); ++j)
        {
            demo_struct->sub2_fixed_array[i].f32_fixed_array[j] = 32.32;
        }
        demo_struct->sub2_fixed_array[i].f64_dynamic_array_len = i;
        demo_struct->sub2_fixed_array[i].f64_dynamic_array = malloc(sizeof(float64_t) * demo_struct->sub2_fixed_array[i].f64_dynamic_array_len);
        for (j = 0; j < (size_t)demo_struct->sub2_fixed_array[i].f64_dynamic_array_len; ++j)
        {
            demo_struct->sub2_fixed_array[i].f64_dynamic_array[j] = 64.64;
        }
    }
}

static void print_demo_struct(const demo_struct_main_t *demo_struct, const char *struct_name)
{
    size_t i, j;

    printf(">>> %s:\n", struct_name);

    printf("i8: %d\n", demo_struct->i8);
    printf("i16: %d\n", demo_struct->i16);
    printf("i32: %d\n", (int)demo_struct->i32);
    printf("i64: %d\n", (int)demo_struct->i64);
    printf("f32: %.6f\n", demo_struct->f32);
    printf("f64: %.6f\n", demo_struct->f64);

    printf("i8_fixed_array:");
    for (i = 0; i < sizeof(demo_struct->i8_fixed_array) / sizeof(int8_t); ++i)
    {
        printf(" %d", demo_struct->i8_fixed_array[i]);
    }
    printf("\n");

    printf("i16_fixed_array:");
    for (i = 0; i < sizeof(demo_struct->i16_fixed_array) / sizeof(int16_t); ++i)
    {
        printf(" %d", demo_struct->i16_fixed_array[i]);
    }
    printf("\n");

    printf("i32_fixed_array:");
    for (i = 0; i < sizeof(demo_struct->i32_fixed_array) / sizeof(int32_t); ++i)
    {
        printf(" %d", demo_struct->i32_fixed_array[i]);
    }
    printf("\n");

    printf("i64_fixed_array:");
    for (i = 0; i < sizeof(demo_struct->i64_fixed_array) / sizeof(int64_t); ++i)
    {
        printf(" %d", (int)demo_struct->i64_fixed_array[i]);
    }
    printf("\n");

    printf("f32_fixed_array:");
    for (i = 0; i < sizeof(demo_struct->f32_fixed_array) / sizeof(float32_t); ++i)
    {
        printf(" %.6f", demo_struct->f32_fixed_array[i]);
    }
    printf("\n");

    printf("f64_fixed_array:");
    for (i = 0; i < sizeof(demo_struct->f64_fixed_array) / sizeof(float64_t); ++i)
    {
        printf(" %.6f", demo_struct->f64_fixed_array[i]);
    }
    printf("\n");

    printf("sub1_fixed_array:\n");
    for (i = 0; i < sizeof(demo_struct->sub1_fixed_array) / sizeof(demo_struct_sub1_t); ++i)
    {
        const demo_struct_sub1_t *sub1 = &demo_struct->sub1_fixed_array[i];

        printf("\t[%d]\n", (int)i + 1);

        printf("\t\ti8_single: %d\n", sub1->i8_single);
        printf("\t\ti16_fixed_array:");
        for (j = 0; j < sizeof(sub1->i16_fixed_array) / sizeof(int16_t); ++j)
        {
            printf(" %d", sub1->i16_fixed_array[j]);
        }
        printf("\n\t\ti32_dynamic_array_len: %d\n", sub1->i32_dynamic_array_len);
        printf("\t\ti32_dynamic_array:");
        for (j = 0; j < (size_t)sub1->i32_dynamic_array_len; ++j)
        {
            printf(" %d", sub1->i32_dynamic_array[j]);
        }
        printf("\n");
    }

    printf("sub1_dynamic_array_len: %d\n", demo_struct->sub1_dynamic_array_len);
    printf("sub1_dynamic_array:\n");
    for (i = 0; i < (size_t)demo_struct->sub1_dynamic_array_len; ++i)
    {
        const demo_struct_sub1_t *sub1 = &demo_struct->sub1_dynamic_array[i];

        printf("\t[%d]\n", (int)i + 1);

        printf("\t\ti8_single: %d\n", sub1->i8_single);
        printf("\t\ti16_fixed_array:");
        for (j = 0; j < sizeof(sub1->i16_fixed_array) / sizeof(int16_t); ++j)
        {
            printf(" %d", sub1->i16_fixed_array[j]);
        }
        printf("\n\t\ti32_dynamic_array_len: %d\n", sub1->i32_dynamic_array_len);
        printf("\t\ti32_dynamic_array:");
        for (j = 0; j < (size_t)sub1->i32_dynamic_array_len; ++j)
        {
            printf(" %d", sub1->i32_dynamic_array[j]);
        }
        printf("\n");
    }

    printf("int_dynamic_array_len: %d\n", demo_struct->int_dynamic_array_len);

    printf("i8_dynamic_array:");
    for (i = 0; i < (size_t)demo_struct->int_dynamic_array_len; ++i)
    {
        printf(" %d", demo_struct->i8_dynamic_array[i]);
    }
    printf("\n");

    printf("i16_dynamic_array:");
    for (i = 0; i < (size_t)demo_struct->int_dynamic_array_len; ++i)
    {
        printf(" %d", demo_struct->i16_dynamic_array[i]);
    }
    printf("\n");

    printf("i32_dynamic_array:");
    for (i = 0; i < (size_t)demo_struct->int_dynamic_array_len; ++i)
    {
        printf(" %d", demo_struct->i32_dynamic_array[i]);
    }
    printf("\n");

    printf("i64_dynamic_array:");
    for (i = 0; i < (size_t)demo_struct->int_dynamic_array_len; ++i)
    {
        printf(" %d", (int)demo_struct->i64_dynamic_array[i]);
    }
    printf("\n");

    printf("f32_dynamic_array_len: %d\n", demo_struct->f32_dynamic_array_len);
    printf("f32_dynamic_array:");
    for (i = 0; i < (size_t)demo_struct->f32_dynamic_array_len; ++i)
    {
        printf(" %.6f", demo_struct->f32_dynamic_array[i]);
    }
    printf("\n");

    printf("f64_dynamic_array_len: %d\n", demo_struct->f64_dynamic_array_len);
    printf("f64_dynamic_array:");
    for (i = 0; i < (size_t)demo_struct->f64_dynamic_array_len; ++i)
    {
        printf(" %.6f", demo_struct->f64_dynamic_array[i]);
    }
    printf("\n");

    printf("sub2_dynamic_array_len: %d\n", demo_struct->sub2_dynamic_array_len);
    printf("sub2_dynamic_array:\n");
    for (i = 0; i < (size_t)demo_struct->sub2_dynamic_array_len; ++i)
    {
        printf("\t[%d]\n", (int)i + 1);

        printf("\t\ti64_single: %d\n", (int)demo_struct->sub2_fixed_array[i].i64_single);
        printf("\t\tf32_fixed_array:");
        for (j = 0; j < sizeof(demo_struct->sub2_fixed_array[i].f32_fixed_array) / sizeof(float32_t); ++j)
        {
            printf(" %.6f", demo_struct->sub2_fixed_array[i].f32_fixed_array[j]);
        }
        printf("\n\t\tf64_dynamic_array_len: %d\n", demo_struct->sub2_fixed_array[i].f64_dynamic_array_len);
        printf("\t\tf64_dynamic_array:");
        for (j = 0; j < (size_t)demo_struct->sub2_fixed_array[i].f64_dynamic_array_len; ++j)
        {
            printf(" %.6f", demo_struct->sub2_fixed_array[i].f64_dynamic_array[j]);
        }
        printf("\n");
    }

    printf("sub2_fixed_array:\n");
    for (i = 0; i < 5; ++i)
    {
        printf("\t[%d]\n", (int)i + 1);

        printf("\t\ti64_single: %d\n", (int)demo_struct->sub2_fixed_array[i].i64_single);
        printf("\t\tf32_fixed_array:");
        for (j = 0; j < sizeof(demo_struct->sub2_fixed_array[i].f32_fixed_array) / sizeof(float32_t); ++j)
        {
            printf(" %.6f", demo_struct->sub2_fixed_array[i].f32_fixed_array[j]);
        }
        printf("\n\t\tf64_dynamic_array_len: %d\n", demo_struct->sub2_fixed_array[i].f64_dynamic_array_len);
        printf("\t\tf64_dynamic_array:");
        for (j = 0; j < (size_t)demo_struct->sub2_fixed_array[i].f64_dynamic_array_len; ++j)
        {
            printf(" %.6f", demo_struct->sub2_fixed_array[i].f64_dynamic_array[j]);
        }
        printf("\n");
    }
}

#define RETURN_IF_NOT_EQUAL(name, val1, val2, fmt)      if ((val1) != (val2)) { \
    fprintf(stderr, "*** struct comparison: " name ": " fmt " != " fmt "\n", (val1), (val2)); \
    return false; \
}

#define RETURN_IF_ARRAY_ITEM_NOT_EQUAL(arr_name, item_name, item_idx, val1, val2, fmt)  \
    if ((val1) != (val2)) { \
    fprintf(stderr, "*** struct comparison: " arr_name "[%d]" item_name ": " fmt " != " fmt "\n", \
        (int)item_idx, (val1), (val2)); \
    return false; \
}

#define RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL(top_arrname, sub_arrname, sub_arridx, item_name, item_idx, val1, val2, fmt) \
    if ((val1) != (val2)) { \
    fprintf(stderr, "*** struct comparison: " top_arrname "[%d]." sub_arrname "[%d]. " item_name ": " fmt " != " fmt "\n", \
        (int)sub_arridx, (int)item_idx, (val1), (val2)); \
    return false; \
}

static bool check_struct_differences(const demo_struct_main_t *struct1, const demo_struct_main_t *struct2)
{
    size_t i, j;

    RETURN_IF_NOT_EQUAL("i8", struct1->i8, struct2->i8, "%d");
    RETURN_IF_NOT_EQUAL("i16", struct1->i16, struct2->i16, "%d");
    RETURN_IF_NOT_EQUAL("i32", (int)struct1->i32, (int)struct2->i32, "%d");
    RETURN_IF_NOT_EQUAL("i64", (int)struct1->i64, (int)struct2->i64, "%d");
    RETURN_IF_NOT_EQUAL("f32", struct1->f32, struct2->f32, "%.6f");
    RETURN_IF_NOT_EQUAL("f64", struct1->f64, struct2->f64, "%.6f");

    for (i = 0; i < sizeof(struct1->i8_fixed_array) / sizeof(int8_t); ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i8_fixed_array", "", i,
            struct1->i8_fixed_array[i], struct2->i8_fixed_array[i], "%d");
    }

    for (i = 0; i < sizeof(struct1->i16_fixed_array) / sizeof(int16_t); ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i16_fixed_array", "", i,
            struct1->i16_fixed_array[i], struct2->i16_fixed_array[i], "%d");
    }

    for (i = 0; i < sizeof(struct1->i32_fixed_array) / sizeof(int32_t); ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i32_fixed_array", "", i,
            (int)struct1->i32_fixed_array[i], (int)struct2->i32_fixed_array[i], "%d");
    }

    for (i = 0; i < sizeof(struct1->i64_fixed_array) / sizeof(int64_t); ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i64_fixed_array", "", i,
            (int)struct1->i64_fixed_array[i], (int)struct2->i64_fixed_array[i], "%d");
    }

    for (i = 0; i < sizeof(struct1->f32_fixed_array) / sizeof(float32_t); ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("f32_fixed_array", "", i,
            struct1->f32_fixed_array[i], struct2->f32_fixed_array[i], "%.6f");
    }

    for (i = 0; i < sizeof(struct1->f64_fixed_array) / sizeof(float64_t); ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("f64_fixed_array", "", i,
            struct1->f64_fixed_array[i], struct2->f64_fixed_array[i], "%.6f");
    }

    for (i = 0; i < sizeof(struct1->sub1_fixed_array) / sizeof(demo_struct_sub1_t); ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub1_fixed_array", ".i8_single", i,
            struct1->sub1_fixed_array[i].i8_single, struct2->sub1_fixed_array[i].i8_single, "%d");

        for (j = 0; j < sizeof(struct1->sub1_fixed_array[i].i16_fixed_array) / sizeof(int16_t); ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub1_fixed_array", "i16_fixed_array", i,
                "", j, struct1->sub1_fixed_array[i].i16_fixed_array[j],
                struct2->sub1_fixed_array[i].i16_fixed_array[j], "%d");
        }

        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub1_fixed_array", ".i32_dynamic_array_len", i,
            struct1->sub1_fixed_array[i].i32_dynamic_array_len,
            struct2->sub1_fixed_array[i].i32_dynamic_array_len, "%d");

        for (j = 0; j < (size_t)struct1->sub1_fixed_array[i].i32_dynamic_array_len; ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub1_fixed_array", "i32_dynamic_array", i,
                "", j, (int)struct1->sub1_fixed_array[i].i32_dynamic_array[j],
                (int)struct2->sub1_fixed_array[i].i32_dynamic_array[j], "%d");
        }
    }

    RETURN_IF_NOT_EQUAL("sub1_dynamic_array_len", struct1->sub1_dynamic_array_len,
        struct2->sub1_dynamic_array_len, "%d");

    for (i = 0; i < (size_t)struct1->sub1_dynamic_array_len; ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub1_dynamic_array", ".i8_single", i,
            struct1->sub1_dynamic_array[i].i8_single, struct2->sub1_dynamic_array[i].i8_single, "%d");

        for (j = 0; j < sizeof(struct1->sub1_dynamic_array[i].i16_fixed_array) / sizeof(int16_t); ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub1_dynamic_array", "i16_fixed_array", i,
                "", j, struct1->sub1_dynamic_array[i].i16_fixed_array[j],
                struct2->sub1_dynamic_array[i].i16_fixed_array[j], "%d");
        }

        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub1_dynamic_array", ".i32_dynamic_array_len", i,
            struct1->sub1_dynamic_array[i].i32_dynamic_array_len,
            struct2->sub1_dynamic_array[i].i32_dynamic_array_len, "%d");

        for (j = 0; j < (size_t)struct1->sub1_dynamic_array[i].i32_dynamic_array_len; ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub1_dynamic_array", "i32_dynamic_array", i,
                "", j, (int)struct1->sub1_dynamic_array[i].i32_dynamic_array[j],
                (int)struct2->sub1_dynamic_array[i].i32_dynamic_array[j], "%d");
        }
    }

    RETURN_IF_NOT_EQUAL("int_dynamic_array_len", struct1->int_dynamic_array_len,
        struct2->int_dynamic_array_len, "%d");

    for (i = 0; i < (size_t)struct1->int_dynamic_array_len; ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i8_dynamic_array", "", i,
            struct1->i8_dynamic_array[i], struct2->i8_dynamic_array[i], "%d");

        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i16_dynamic_array", "", i,
            struct1->i16_dynamic_array[i], struct2->i16_dynamic_array[i], "%d");

        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i32_dynamic_array", "", i,
            (int)struct1->i32_dynamic_array[i], (int)struct2->i32_dynamic_array[i], "%d");

        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("i64_dynamic_array", "", i,
            (int)struct1->i64_dynamic_array[i], (int)struct2->i64_dynamic_array[i], "%d");
    }

    RETURN_IF_NOT_EQUAL("f32_dynamic_array_len", struct1->f32_dynamic_array_len,
        struct2->f32_dynamic_array_len, "%d");

    for (i = 0; i < (size_t)struct1->f32_dynamic_array_len; ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("f32_dynamic_array", "", i,
            struct1->f32_dynamic_array[i], struct2->f32_dynamic_array[i], "%.6f");
    }

    RETURN_IF_NOT_EQUAL("f64_dynamic_array_len", struct1->f64_dynamic_array_len,
        struct2->f64_dynamic_array_len, "%d");

    for (i = 0; i < (size_t)struct1->f64_dynamic_array_len; ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("f64_dynamic_array", "", i,
            struct1->f64_dynamic_array[i], struct2->f64_dynamic_array[i], "%.6f");
    }

    RETURN_IF_NOT_EQUAL("sub2_dynamic_array_len", struct1->sub2_dynamic_array_len,
        struct2->sub2_dynamic_array_len, "%d");

    for (i = 0; i < (size_t)struct1->sub2_dynamic_array_len; ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub2_dynamic_array", ".i64_single", i,
            (int)struct1->sub2_dynamic_array[i].i64_single, (int)struct2->sub2_dynamic_array[i].i64_single, "%d");

        for (j = 0; j < sizeof(struct1->sub2_dynamic_array[i].f32_fixed_array) / sizeof(float32_t); ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub2_dynamic_array", "f32_fixed_array", i,
                "", j, struct1->sub2_dynamic_array[i].f32_fixed_array[j],
                struct2->sub2_dynamic_array[i].f32_fixed_array[j], "%.6f");
        }

        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub2_dynamic_array", ".f64_dynamic_array_len", i,
            struct1->sub2_dynamic_array[i].f64_dynamic_array_len,
            struct2->sub2_dynamic_array[i].f64_dynamic_array_len, "%d");

        for (j = 0; j < (size_t)struct1->sub2_dynamic_array[i].f64_dynamic_array_len; ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub2_dynamic_array", "f64_dynamic_array", i,
                "", j, struct1->sub2_dynamic_array[i].f64_dynamic_array[j],
                struct2->sub2_dynamic_array[i].f64_dynamic_array[j], "%.6f");
        }
    }

    for (i = 0; i < 5; ++i)
    {
        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub2_fixed_array", ".i64_single", i,
            (int)struct1->sub2_fixed_array[i].i64_single, (int)struct2->sub2_fixed_array[i].i64_single, "%d");

        for (j = 0; j < sizeof(struct1->sub2_fixed_array[i].f32_fixed_array) / sizeof(float32_t); ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub2_fixed_array", "f32_fixed_array", i,
                "", j, struct1->sub2_fixed_array[i].f32_fixed_array[j],
                struct2->sub2_fixed_array[i].f32_fixed_array[j], "%.6f");
        }

        RETURN_IF_ARRAY_ITEM_NOT_EQUAL("sub2_fixed_array", ".f64_dynamic_array_len", i,
            struct1->sub2_fixed_array[i].f64_dynamic_array_len,
            struct2->sub2_fixed_array[i].f64_dynamic_array_len, "%d");

        for (j = 0; j < (size_t)struct1->sub2_fixed_array[i].f64_dynamic_array_len; ++j)
        {
            RETURN_IF_2D_ARRAY_ITEM_NOT_EQUAL("sub2_fixed_array", "f64_dynamic_array", i,
                "", j, struct1->sub2_fixed_array[i].f64_dynamic_array[j],
                struct2->sub2_fixed_array[i].f64_dynamic_array[j], "%.6f");
        }
    }

    return true;
}

static bool check_buffer_differences(const uint8_t *buf1, const uint8_t *buf2, uint16_t size)
{
    uint16_t i = 0;

    for (; i < size; ++i)
    {
        if (buf1[i] != buf2[i])
        {
            char buf1_dumped_str[4096] = { 0 };

            fprintf(stderr, "*** buf1 differs from buf2 at index %d: %d vs %d\n", i, buf1[i], buf2[i]);
            commproto_dump_buffer(buf1, size, NULL, buf1_dumped_str);
            fprintf(stderr, ">>> buf1:\n%s", buf1_dumped_str);
            fprintf(stderr, ">>> buf2:\n");
            commproto_dump_buffer(buf2, size, stderr, NULL);

            return false;
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    demo_struct_main_t src = { 0 };
    uint8_t buf[4096] = { 0 };
    demo_struct_main_t dest1 = { 0 };
    demo_struct_main_t dest2 = { 0 };
    commproto_result_t result;

    if ((result.error_code = commproto_init()) < 0)
    {
        fprintf(stderr, "*** Communication facility initialization failed: %s!\n", commproto_error(result.error_code));

        return -1;
    }

    if (NULL == getenv("COMMPROTO_DEBUG"))
        printf("\n!!! NOTE: To output more details, run (in bash, for example): COMMPROTO_DEBUG=1 %s !!!\n\n", argv[0]);

    printf("COMMPROTO_META_SIZE(demo_struct_main_t) = %d, sizeof(demo_struct_main_t) = %d,"
        " sizeof(demo_struct_sub1_t) = %d, sizeof(demo_struct_sub2_t)[] = %d\n",
        (int)COMMPROTO_META_SIZE(demo_struct_main_t), (int)sizeof(demo_struct_main_t),
        (int)sizeof(demo_struct_sub1_t), (int)sizeof(src.sub2_fixed_array));

    COMMPROTO_DPRINT("src struct addr = %p, buf addr = %p, dest1 struct addr = %p, dest2 struct addr = %p\n",
        (void *)&src, buf, (void *)&dest1, (void *)&dest2);

    fill_demo_struct(&src);
    print_demo_struct(&src, "src struct");

    result = COMMPROTO_SERIALIZE(demo_struct_main_t, &src, buf, sizeof(buf));
    if (result.error_code < 0)
    {
        fprintf(stderr, "*** Data serialization to static buffer failed after %u bytes: %s!\n",
            result.handled_len, commproto_error(result.error_code));
        COMMPROTO_CLEAR(demo_struct_main_t, &src);

        return -1;
    }
    printf("Serialized %u bytes to static buffer.\n", result.handled_len);
    commproto_dump_buffer(buf, result.handled_len, stdout, NULL);

    result = COMMPROTO_PARSE(demo_struct_main_t, buf, result.handled_len, &dest1);
    if (result.error_code < 0)
    {
        fprintf(stderr, "*** Data deserialization from static buffer failed after %u bytes: %s!\n",
            result.handled_len, commproto_error(result.error_code));
        COMMPROTO_CLEAR(demo_struct_main_t, &src);
        COMMPROTO_CLEAR(demo_struct_main_t, &dest1);

        return -1;
    }
    printf("Deserialized %u bytes from static buffer.\n", result.handled_len);
    print_demo_struct(&dest1, "dest1 struct");

    if (!check_struct_differences(&src, &dest1))
    {
        COMMPROTO_CLEAR(demo_struct_main_t, &src);
        COMMPROTO_CLEAR(demo_struct_main_t, &dest1);

        return -1;
    }

    result = COMMPROTO_SERIALIZE(demo_struct_main_t, &dest1, NULL, 0);
    if (NULL == result.buf_ptr || result.error_code < 0)
    {
        fprintf(stderr, "*** Data serialization to dynamic buffer failed after %u bytes: %s!\n",
            result.handled_len, commproto_error(result.error_code));
        COMMPROTO_CLEAR(demo_struct_main_t, &src);
        COMMPROTO_CLEAR(demo_struct_main_t, &dest1);
        free(result.buf_ptr);

        return -1;
    }
    printf("Serialized %u bytes to dynamic buffer.\n", result.handled_len);

    COMMPROTO_CLEAR(demo_struct_main_t, &dest1);

    result = COMMPROTO_PARSE(demo_struct_main_t, result.buf_ptr, result.handled_len, &dest2);
    if (result.error_code < 0)
    {
        fprintf(stderr, "*** Data deserialization from dynamic buffer failed after %d bytes: %s!\n",
            result.error_code, commproto_error(result.error_code));
        COMMPROTO_CLEAR(demo_struct_main_t, &src);
        COMMPROTO_CLEAR(demo_struct_main_t, &dest2);
        free(result.buf_ptr);

        return -1;
    }
    printf("Deserialized %d bytes from dynamic buffer.\n", result.error_code);
    print_demo_struct(&dest2, "dest2 struct");

    if (!check_buffer_differences(buf, result.buf_ptr, result.handled_len))
    {
        COMMPROTO_CLEAR(demo_struct_main_t, &src);
        COMMPROTO_CLEAR(demo_struct_main_t, &dest2);
        free(result.buf_ptr);

        return -1;
    }

    free(result.buf_ptr);

    if (!check_struct_differences(&src, &dest2))
    {
        COMMPROTO_CLEAR(demo_struct_main_t, &src);
        COMMPROTO_CLEAR(demo_struct_main_t, &dest2);

        return -1;
    }

    COMMPROTO_CLEAR(demo_struct_main_t, &src);
    COMMPROTO_CLEAR(demo_struct_main_t, &dest2);

    printf("~ ~ ~ ~ Test finished successfully! ~ ~ ~ ~\n");

    return 0;
}

#endif /* #ifdef TEST */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __COMMUNICATION_PROTOCOL_H__ */

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
 * >>> 2022-02-27, Man Hung-Coeng:
 *  01. Fix the alignment fault problem on some platforms (ARM, for example).
 *
 * >>> 2022-03-05, Man Hung-Coeng:
 *  01. Add hints of how to define macro __ORDER_LITTLE_ENDIAN__,
 *      __ORDER_BIG_ENDIAN__ and __BYTE_ORDER__.
 *
 * >>> 2022-03-08, Man Hung-Coeng:
 *  01. Remove the struct_name argument from macro COMMPROTO_CPP_SERIALIZE()
 *      and COMMPROTO_CPP_PARSE().
 *
 * >>> 2022-05-06, Man Hung-Coeng:
 *  01. Add function commproto_clear() and macro COMMPROTO_DEFINE_DESTRUCTOR()
 *      for struct memory release.
 *
 * >>> 2022-05-07, Man Hung-Coeng:
 *  01. Add macro COMMPROTO_CLEAR() and COMMPROTO_CPP_CLEAR().
 *  02. Change the type of return value of commproto_serialize() and
 *      commproto_parse() to commproto_result_t (a new added type)
 *      in order to reduce the amount of function parameters.
 *  03. Redefine enumerated values of field types, e.g., COMMPROTO_INT8,
 *      to establish stronger relations between field types and their sizes,
 *      which makes it possible to merge some boring conditional statements
 *      in functions using them.
 *
 * >>> 2022-05-08, Man Hung-Coeng:
 *  01. Change types of *_len parameters/fields from [u]int16_t to [u]int32_t.
 *  02. Refactor.
 *
 * >>> 2022-05-09, Man Hung-Coeng:
 *  01. Add macro COMMPROTO_CPP_VSERIALIZE(), COMMPROTO_CPP_VPARSE() and
 *      COMMPROTO_CPP_VCLEAR() for structs CONTAINING virtual functions!
 */

