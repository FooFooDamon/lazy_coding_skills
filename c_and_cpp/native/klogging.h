/*
 * Linux kernel logging wrappers.
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

#ifndef __KLOGGING_H__
#define __KLOGGING_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KBUILD_MODNAME
#error KBUILD_MODNAME not defined!
#endif

#ifndef __DEVNAME__
#define __DEVNAME__                                     KBUILD_MODNAME
#endif

/* "v" is short for "verbose". */

#define __print_logging_v(level, format, ...)	    \
	pr_##level(__DEVNAME__ ": " __FILE__ ":%d %s(): " format, \
		__LINE__, __func__, ##__VA_ARGS__)

#define __print_logging_ratelimited_v(level, format, ...)	    \
	pr_##level##_ratelimited(__DEVNAME__ ": " __FILE__ ":%d %s(): " format, \
		__LINE__, __func__, ##__VA_ARGS__)

/* Macros below need <linux/printk.h>. */
#define pr_emerg_v(format, ...)                         __print_logging_v(emerg, format, ##__VA_ARGS__)
#define pr_alert_v(format, ...)                         __print_logging_v(alert, format, ##__VA_ARGS__)
#define pr_crit_v(format, ...)                          __print_logging_v(crit, format, ##__VA_ARGS__)
#define pr_err_v(format, ...)                           __print_logging_v(err, format, ##__VA_ARGS__)
#define pr_warn_v(format, ...)                          __print_logging_v(warn, format, ##__VA_ARGS__)
#define pr_notice_v(format, ...)                        __print_logging_v(notice, format, ##__VA_ARGS__)
#define pr_info_v(format, ...)                          __print_logging_v(info, format, ##__VA_ARGS__)
#define pr_cont_v(format, ...)                          __print_logging_v(cont, format, ##__VA_ARGS__)
#define pr_devel_v(format, ...)                         __print_logging_v(devel, format, ##__VA_ARGS__)
#define pr_debug_v(format, ...)                         __print_logging_v(debug, format, ##__VA_ARGS__)
#define pr_emerg_ratelimited_v(format, ...)             __print_logging_ratelimited_v(emerg, format, ##__VA_ARGS__)
#define pr_alert_ratelimited_v(format, ...)             __print_logging_ratelimited_v(alert, format, ##__VA_ARGS__)
#define pr_crit_ratelimited_v(format, ...)              __print_logging_ratelimited_v(crit, format, ##__VA_ARGS__)
#define pr_err_ratelimited_v(format, ...)               __print_logging_ratelimited_v(err, format, ##__VA_ARGS__)
#define pr_warn_ratelimited_v(format, ...)              __print_logging_ratelimited_v(warn, format, ##__VA_ARGS__)
#define pr_notice_ratelimited_v(format, ...)            __print_logging_ratelimited_v(notice, format, ##__VA_ARGS__)
#define pr_info_ratelimited_v(format, ...)              __print_logging_ratelimited_v(info, format, ##__VA_ARGS__)
/*#define pr_cont_ratelimited_v(format, ...)              __print_logging_ratelimited_v(cont, format, ##__VA_ARGS__)*/
#define pr_devel_ratelimited_v(format, ...)             __print_logging_ratelimited_v(devel, format, ##__VA_ARGS__)
#define pr_debug_ratelimited_v(format, ...)             __print_logging_ratelimited_v(debug, format, ##__VA_ARGS__)

#define __device_logging_v(prefix, dev, level, format, ...)	    \
	prefix##_##level(dev, __DEVNAME__ ": " __FILE__ ":%d %s(): " format, \
		__LINE__, __func__, ##__VA_ARGS__)

#define __device_logging_ratelimited_v(prefix, dev, level, format, ...)	    \
	prefix##_##level##_ratelimited(dev, __DEVNAME__ ": " __FILE__ ":%d %s(): " format, \
		__LINE__, __func__, ##__VA_ARGS__)

/* Macros below need <linux/netdevice.h> and <linux/net.h>. */
#define netdev_emerg_v(dev, format, ...)                __device_logging_v(netdev, dev, emerg, format, ##__VA_ARGS__)
#define netdev_alert_v(dev, format, ...)                __device_logging_v(netdev, dev, alert, format, ##__VA_ARGS__)
#define netdev_crit_v(dev, format, ...)                 __device_logging_v(netdev, dev, crit, format, ##__VA_ARGS__)
#define netdev_err_v(dev, format, ...)                  __device_logging_v(netdev, dev, err, format, ##__VA_ARGS__)
#define netdev_warn_v(dev, format, ...)                 __device_logging_v(netdev, dev, warn, format, ##__VA_ARGS__)
#define netdev_notice_v(dev, format, ...)               __device_logging_v(netdev, dev, notice, format, ##__VA_ARGS__)
#define netdev_info_v(dev, format, ...)                 __device_logging_v(netdev, dev, info, format, ##__VA_ARGS__)
#define netdev_debug_v(dev, format, ...)                __device_logging_v(netdev, dev, dbg, format, ##__VA_ARGS__)
#define netdev_emerg_ratelimited_v(dev, format, ...)    net_ratelimited_function(netdev_emerg, dev, format, ##__VA_ARGS__)
#define netdev_alert_ratelimited_v(dev, format, ...)    net_ratelimited_function(netdev_alert, dev, format, ##__VA_ARGS__)
#define netdev_crit_ratelimited_v(dev, format, ...)     net_ratelimited_function(netdev_crit, dev, format, ##__VA_ARGS__)
#define netdev_err_ratelimited_v(dev, format, ...)      net_ratelimited_function(netdev_err, dev, format, ##__VA_ARGS__)
#define netdev_warn_ratelimited_v(dev, format, ...)     net_ratelimited_function(netdev_warn, dev, format, ##__VA_ARGS__)
#define netdev_notice_ratelimited_v(dev, format, ...)   net_ratelimited_function(netdev_notice, dev, format, ##__VA_ARGS__)
#define netdev_info_ratelimited_v(dev, format, ...)     net_ratelimited_function(netdev_info, dev, format, ##__VA_ARGS__)
#define netdev_debug_ratelimited_v(dev, format, ...)    net_ratelimited_function(netdev_dbg, dev, format, ##__VA_ARGS__)

/* Macros below need <linux/device.h>. */
#define dev_emerg_v(_dev_, format, ...)                 __device_logging_v(dev, _dev_, emerg, format, ##__VA_ARGS__)
#define dev_alert_v(_dev_, format, ...)                 __device_logging_v(dev, _dev_, alert, format, ##__VA_ARGS__)
#define dev_crit_v(_dev_, format, ...)                  __device_logging_v(dev, _dev_, crit, format, ##__VA_ARGS__)
#define dev_err_v(_dev_, format, ...)                   __device_logging_v(dev, _dev_, err, format, ##__VA_ARGS__)
#define dev_warn_v(_dev_, format, ...)                  __device_logging_v(dev, _dev_, warn, format, ##__VA_ARGS__)
#define dev_notice_v(_dev_, format, ...)                __device_logging_v(dev, _dev_, notice, format, ##__VA_ARGS__)
#define dev_info_v(_dev_, format, ...)                  __device_logging_v(dev, _dev_, info, format, ##__VA_ARGS__)
#define dev_debug_v(_dev_, format, ...)                 __device_logging_v(dev, _dev_, dbg, format, ##__VA_ARGS__)
#define dev_emerg_ratelimited_v(_dev_, format, ...)     __device_logging_ratelimited_v(dev, _dev_, emerg, format, ##__VA_ARGS__)
#define dev_alert_ratelimited_v(_dev_, format, ...)     __device_logging_ratelimited_v(dev, _dev_, alert, format, ##__VA_ARGS__)
#define dev_crit_ratelimited_v(_dev_, format, ...)      __device_logging_ratelimited_v(dev, _dev_, crit, format, ##__VA_ARGS__)
#define dev_err_ratelimited_v(_dev_, format, ...)       __device_logging_ratelimited_v(dev, _dev_, err, format, ##__VA_ARGS__)
#define dev_warn_ratelimited_v(_dev_, format, ...)      __device_logging_ratelimited_v(dev, _dev_, warn, format, ##__VA_ARGS__)
#define dev_notice_ratelimited_v(_dev_, format, ...)    __device_logging_ratelimited_v(dev, _dev_, notice, format, ##__VA_ARGS__)
#define dev_info_ratelimited_v(_dev_, format, ...)      __device_logging_ratelimited_v(dev, _dev_, info, format, ##__VA_ARGS__)
#define dev_debug_ratelimited_v(_dev_, format, ...)     __device_logging_ratelimited_v(dev, _dev_, dbg, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __KLOGGING_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-09-26, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 */

