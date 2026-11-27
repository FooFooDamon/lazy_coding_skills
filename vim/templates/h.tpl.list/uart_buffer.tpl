/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Buffer structures and interfaces for UART.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 *
 * This file is initially generated from:
 *      vim/templates/h.tpl.list/uart_buffer.tpl
 * of:
 *      https://github.com/FooFooDamon/lazy_coding_skills
 */

#ifndef __${HEADER_GUARD}_H__
#define __${HEADER_GUARD}_H__

#ifndef UART_BUF_SIZE_SMALL
#define UART_BUF_SIZE_SMALL         64
#endif

#define RING_BUF_SIZE_SMALL         UART_BUF_SIZE_SMALL

#ifndef UART_BUF_SIZE_MEDIUM
#define UART_BUF_SIZE_MEDIUM        256
#endif

#define RING_BUF_SIZE_MEDIUM        UART_BUF_SIZE_MEDIUM

#include "ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef big_ring_buffer_t                                   uart_big_buffer_t;

typedef medium_ring_buffer_t                                uart_medium_buffer_t;

typedef small_ring_buffer_t                                 uart_small_buffer_t;

typedef RING_BUF_STRUCT_TEMPLATE(uart_empty_buffer, 0)      uart_empty_buffer_t;

#define UART_BUF_INITIALIZER()                              { 0, 0, { 0 }, { 0, 0x03 } }

#define UART_BUF_CAPACITY                                   RING_BUF_CAPACITY

#define UART_BUF_CHECK_EMPTY                                RING_BUF_CHECK_EMPTY
#define UART_BUF_IS_EMPTY                                   RING_BUF_IS_EMPTY

#define UART_BUF_CHECK_FULL                                 RING_BUF_CHECK_FULL
#define UART_BUF_IS_FULL                                    RING_BUF_IS_FULL

#define UART_BUF_CALC_TOTAL_FREE_SIZE                       RING_BUF_CALC_TOTAL_FREE_SIZE
#define UART_BUF_TOTAL_FREE_SIZE                            RING_BUF_TOTAL_FREE_SIZE

#define UART_BUF_CALC_CONTINUOUS_FREE_SIZE                  RING_BUF_CALC_CONTINUOUS_FREE_SIZE
#define UART_BUF_CONTINUOUS_FREE_SIZE                       RING_BUF_CONTINUOUS_FREE_SIZE

#define UART_BUF_CALC_OCCUPIED_SIZE                         RING_BUF_CALC_OCCUPIED_SIZE
#define UART_BUF_OCCUPIED_SIZE                              RING_BUF_OCCUPIED_SIZE

#define UART_BUF_CALC_FORWARDED_INDEX                       RING_BUF_CALC_FORWARDED_INDEX
#define UART_BUF_HEAD_FORWARD                               RING_BUF_HEAD_FORWARD
#define UART_BUF_TAIL_FORWARD                               RING_BUF_TAIL_FORWARD

#define UART_BUF_RX_IDLE_BIT_MASK                           (1U << 0)
#define UART_BUF_SET_RX_IDLE_BIT(_buf_ptr_)                 (_buf_ptr_)->extra.flag_bits |= UART_BUF_RX_IDLE_BIT_MASK
#define UART_BUF_CLEAR_RX_IDLE_BIT(_buf_ptr_)               (_buf_ptr_)->extra.flag_bits &= ~UART_BUF_RX_IDLE_BIT_MASK
#define UART_BUF_IS_RX_IDLE(_buf_ptr_)                      ((_buf_ptr_)->extra.flag_bits & UART_BUF_RX_IDLE_BIT_MASK)

#define UART_BUF_TX_IDLE_BIT_MASK                           (1U << 1)
#define UART_BUF_SET_TX_IDLE_BIT(_buf_ptr_)                 (_buf_ptr_)->extra.flag_bits |= UART_BUF_TX_IDLE_BIT_MASK
#define UART_BUF_CLEAR_TX_IDLE_BIT(_buf_ptr_)               (_buf_ptr_)->extra.flag_bits &= ~UART_BUF_TX_IDLE_BIT_MASK
#define UART_BUF_IS_TX_IDLE(_buf_ptr_)                      ((_buf_ptr_)->extra.flag_bits & UART_BUF_TX_IDLE_BIT_MASK)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __${HEADER_GUARD}_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
