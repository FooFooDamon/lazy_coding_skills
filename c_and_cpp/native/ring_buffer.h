/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Ring buffer interfaces.
 *
 * Copyright (c) 2026 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 *
 * This file is initially released as c_and_cpp/native/ring_buffer.h of:
 *      https://github.com/FooFooDamon/lazy_coding_skills
 */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RING_BUF_SIZE_SMALL
#define RING_BUF_SIZE_SMALL                         16
#endif

#if RING_BUF_SIZE_SMALL != 16 && RING_BUF_SIZE_SMALL != 32 && RING_BUF_SIZE_SMALL != 64 && RING_BUF_SIZE_SMALL != 128
#error RING_BUF_SIZE_SMALL MUST be 16, 32, 64 or 128!
#endif

#ifndef RING_BUF_SIZE_MEDIUM
#define RING_BUF_SIZE_MEDIUM                        256
#endif

#if RING_BUF_SIZE_MEDIUM != 256 && RING_BUF_SIZE_MEDIUM != 512 && RING_BUF_SIZE_MEDIUM != 1024 && RING_BUF_SIZE_MEDIUM != 2048
#error RING_BUF_SIZE_MEDIUM MUST be 256, 512, 1024 or 2048!
#endif

#ifndef RING_BUF_SIZE_BIG
#define RING_BUF_SIZE_BIG                           4096
#endif

#if RING_BUF_SIZE_BIG != 4096 && RING_BUF_SIZE_BIG != 8192 && RING_BUF_SIZE_BIG != 16384 && RING_BUF_SIZE_BIG != 32768
#error RING_BUF_SIZE_BIG MUST be 4096, 8192, 16384 or 32768!
#endif

#if RING_BUF_SIZE_SMALL > RING_BUF_SIZE_MEDIUM || RING_BUF_SIZE_SMALL > RING_BUF_SIZE_BIG
#error RING_BUF_SIZE_SMALL MUST be smaller than RING_BUF_SIZE_MEDIUM and RING_BUF_SIZE_BIG!
#endif

#if RING_BUF_SIZE_MEDIUM > RING_BUF_SIZE_BIG
#error RING_BUF_SIZE_MEDIUM MUST be smaller than RING_BUF_SIZE_BIG!
#endif

#ifndef RING_BUF_STRUCT_TEMPLATE

#define RING_BUF_STRUCT_TEMPLATE(_name_, _size_)    struct _name_ { \
    volatile uint16_t head; \
    volatile uint16_t tail; \
    union { \
        volatile uint16_t handling; \
        volatile uint16_t aux; \
    } len; \
    struct { \
        volatile uint8_t err_count; \
        volatile uint8_t flag_bits; \
    } extra; \
    uint8_t data[_size_]; \
}

#endif

#ifndef RING_BUF_INITIALIZER
#define RING_BUF_INITIALIZER()                      { 0, 0, { 0 }, { 0 } }
#endif

typedef RING_BUF_STRUCT_TEMPLATE(big_ring_buffer, RING_BUF_SIZE_BIG)        big_ring_buffer_t;

typedef RING_BUF_STRUCT_TEMPLATE(medium_ring_buffer, RING_BUF_SIZE_MEDIUM)  medium_ring_buffer_t;

typedef RING_BUF_STRUCT_TEMPLATE(small_ring_buffer, RING_BUF_SIZE_SMALL)    small_ring_buffer_t;

#define RING_BUF_CAPACITY(_buf_ptr_)                (sizeof((_buf_ptr_)->data) - 1)

#define RING_BUF_CHECK_EMPTY(_head_, _tail_)        ((_head_) == (_tail_))
#define RING_BUF_IS_EMPTY(_buf_ptr_)                RING_BUF_CHECK_EMPTY((_buf_ptr_)->head, (_buf_ptr_)->tail)

#define RING_BUF_CHECK_FULL(_head_, _tail_)         (((_head_) <= (_tail_)) ? 0 : ((_head_) == ((_tail_) + 1)))
#define RING_BUF_IS_FULL(_buf_ptr_)                 RING_BUF_CHECK_FULL((_buf_ptr_)->head, (_buf_ptr_)->tail)

#define RING_BUF_CALC_TOTAL_FREE_SIZE(_max_size_, _head_, _tail_)       \
    (((_head_) <= (_tail_)) ? ((_max_size_) - 1 - (_tail_) + (_head_)) : ((_head_) - 1 - (_tail_)))

#define RING_BUF_TOTAL_FREE_SIZE(_buf_ptr_)         RING_BUF_CALC_TOTAL_FREE_SIZE(sizeof((_buf_ptr_)->data), (_buf_ptr_)->head, (_buf_ptr_)->tail)

#define RING_BUF_CALC_CONTINUOUS_FREE_SIZE(_max_size_, _head_, _tail_)  \
    ((((_head_) <= (_tail_)) ? (_max_size_) : ((_head_) - 1)) - (_tail_))

#define RING_BUF_CONTINUOUS_FREE_SIZE(_buf_ptr_)    RING_BUF_CALC_CONTINUOUS_FREE_SIZE(sizeof((_buf_ptr_)->data), (_buf_ptr_)->head, (_buf_ptr_)->tail)

#define RING_BUF_CALC_OCCUPIED_SIZE(_max_size_, _head_, _tail_)         \
    ((((_head_) <= (_tail_)) ? (_tail_) : ((_tail_) + (_max_size_))) - (_head_))

#define RING_BUF_OCCUPIED_SIZE(_buf_ptr_)           RING_BUF_CALC_OCCUPIED_SIZE(sizeof((_buf_ptr_)->data), (_buf_ptr_)->head, (_buf_ptr_)->tail)

#define RING_BUF_CALC_FORWARDED_INDEX(_max_size_, _cur_idx_, _steps_)   \
    (((_cur_idx_) + (_steps_)) & (_max_size_ - 1))

#define RING_BUF_HEAD_FORWARD(_buf_ptr_, _steps_)   (_buf_ptr_)->head = RING_BUF_CALC_FORWARDED_INDEX(sizeof((_buf_ptr_)->data), (_buf_ptr_)->head, _steps_)

#define RING_BUF_TAIL_FORWARD(_buf_ptr_, _steps_)   (_buf_ptr_)->tail = RING_BUF_CALC_FORWARDED_INDEX(sizeof((_buf_ptr_)->data), (_buf_ptr_)->tail, _steps_)

#ifdef __cplusplus
}
#endif

#endif /* __RING_BUFFER_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2026-11-15, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 *
 * >>> 2026-11-27, Man Hung-Coeng <udc577@126.com>:
 *  01. Update RING_BUF_STRUCT_TEMPLATE(): Change the type of "extra" field
 *      from union to struct.
 */

