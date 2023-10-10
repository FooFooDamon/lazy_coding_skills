/*
 * Evolving Linux kernel interfaces and definitions.
 *
 * Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
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

#ifndef __EVOL_KERNEL_H__
#define __EVOL_KERNEL_H__

#include <linux/version.h>

#ifdef __cplusplus
extern "C" {
#endif

/* <linux/timer.h> */
/* #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0) */
#ifdef setup_timer
typedef unsigned long timer_cb_arg_t;
#define evol_setup_timer(timer, callback, arg_for_old)      setup_timer(timer, callback, (timer_cb_arg_t)(arg_for_old))
#else
struct timer_list;
typedef struct timer_list * timer_cb_arg_t;
#define evol_setup_timer(timer, callback, arg_for_old)      timer_setup(timer, callback, /* flags = */0)
#endif

/* <linux/netdevice.h> */
/* FIXME: Not so sure about the accurate KERNEL_VERSION. */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 1, 15)
#define evol_netdev_open(dev, ext_ack)                      dev_open(dev)
#define evol_netif_trans_update(dev)                        (dev)->trans_start = jiffies
#else
#define evol_netdev_open(dev, ext_ack)                      dev_open(dev, ext_ack)
#define evol_netif_trans_update(dev)                        netif_trans_update(dev)
#endif

/* <linux/can/skb.h> */
/* FIXME: Not so sure about the accurate KERNEL_VERSION. */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 1, 15)
#define evol_can_get_echo_skb(dev, idx, frame_len_ptr)      can_get_echo_skb(dev, idx)
#define evol_can_put_echo_skb(skb, dev, idx, frame_len)     can_put_echo_skb(skb, dev, idx)
#define evol_can_free_echo_skb(dev, idx, frame_len_ptr)     can_free_echo_skb(dev, idx)
#else
#define evol_can_get_echo_skb(dev, idx, frame_len_ptr)      can_get_echo_skb(dev, idx, frame_len_ptr)
#define evol_can_put_echo_skb(skb, dev, idx, frame_len)     can_put_echo_skb(skb, dev, idx, frame_len)
#define evol_can_free_echo_skb(dev, idx, frame_len_ptr)     can_free_echo_skb(dev, idx, frame_len_ptr)
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __EVOL_KERNEL_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-10-10, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 */

