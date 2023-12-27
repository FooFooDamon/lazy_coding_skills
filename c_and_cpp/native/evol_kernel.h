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
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
#define evol_netdev_open(dev, ext_ack)                      dev_open(dev)
#else
#define evol_netdev_open(dev, ext_ack)                      dev_open(dev, ext_ack)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0)
#define evol_netif_trans_update(dev)                        (dev)->trans_start = jiffies
#else
#define evol_netif_trans_update(dev)                        netif_trans_update(dev)
#endif

/* <linux/can/dev.h> (older) or <linux/can/skb.h> (newer) */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
#define evol_can_get_echo_skb(dev, idx, frame_len_ptr)      can_get_echo_skb(dev, idx)
#define evol_can_put_echo_skb(skb, dev, idx, frame_len)     can_put_echo_skb(skb, dev, idx)
#else
#define evol_can_get_echo_skb(dev, idx, frame_len_ptr)      can_get_echo_skb(dev, idx, frame_len_ptr)
#define evol_can_put_echo_skb(skb, dev, idx, frame_len)     can_put_echo_skb(skb, dev, idx, frame_len)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0)
#define evol_can_free_echo_skb(dev, idx, frame_len_ptr)     can_free_echo_skb(dev, idx)
#else
#define evol_can_free_echo_skb(dev, idx, frame_len_ptr)     can_free_echo_skb(dev, idx, frame_len_ptr)
#endif

/* <linux/uaccess.h> or <asm/uaccess.h> */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#define evol_access_ok(addr, size)                          access_ok(addr, size)
#else
#define evol_access_ok(addr, size)                          access_ok(0, addr, size)
#endif

/* <linux/time.h> */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
#define evol_time_to_tm                                     time64_to_tm
#else
#define evol_time_to_tm                                     time_to_tm
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
 *
 * >>> 2023-11-20, Man Hung-Coeng <udc577@126.com>:
 *  01. Make the value of dividing version of evol_can_*_echo_skb() more precise
 *      by changing it from 4.1.15 to 5.{11,12}.0.
 *
 * >>> 2023-12-06, Man Hung-Coeng <udc577@126.com>:
 *  01. Add evol_access_ok().
 *
 * >>> 2023-12-27, Man Hung-Coeng <udc577@126.com>:
 *  01. Amend boundary versions for dev_open(), netif_trans_update()
 *      and can_*_echo_skb().
 *  02. Add evol_time_to_tm().
 */

