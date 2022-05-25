/*
 * Convenient logger interfaces based on syslog mechanism.
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

#ifndef __LOGGER_ON_SYSLOG_H__
#define __LOGGER_ON_SYSLOG_H__

#if 0
#include <time.h>
#include <syslog.h>
#else
/* Too noisy, disable it! */
/*#pragma message("Please include <time.h> and <syslog.h> manually in your source code file if needed.")*/
#endif

/*
 * The two macros below are for CPU branch prediction hints.
 * Redefine them depending on your compiler, before including this header.
 */

#ifndef likely
#pragma message("Macro likely(x) not defined, CPU branch prediction not available.")
#define likely(x)                                   (x)
#endif

#ifndef unlikely
#pragma message("Macro unlikely(x) not defined, CPU branch prediction not available.")
#define unlikely(x)                                 (x)
#endif

extern int __log_option_flags;
extern int __log_facility;
extern unsigned char __log_level_mask;

#define OPEN_SYSLOG(ident, option, facility, level_mask)    do { \
    __log_option_flags = option; \
    __log_facility = facility; \
    __log_level_mask = level_mask; \
    setlogmask(__log_level_mask); \
    openlog(ident, __log_option_flags, __log_facility); \
} while (0)

#define CLOSE_SYSLOG()                              do { \
    closelog(); \
    __log_level_mask = 0xff; \
    __log_facility = LOG_USER; \
    __log_option_flags = 0; \
} while (0)

#define __SYSLOG(level, lv, format, ...)            \
    struct timespec lv##_log_time; \
    clock_gettime(CLOCK_REALTIME, &lv##_log_time); \
    syslog(__log_facility | LOG_##level, #lv " %ld.%09ld " format, \
        lv##_log_time.tv_sec, lv##_log_time.tv_nsec, ##__VA_ARGS__); \

#define __LOGF(level, lv, format, ...)              do { \
    __SYSLOG(level, lv, format, ##__VA_ARGS__); \
} while (0)

#define __VLOGF(level, lv, format, ...)             \
    __LOGF(level, lv, __FILE__ ":%d %s(): " format, __LINE__, __func__, ##__VA_ARGS__)

#define ALLOW_LOG_LEVEL(level)                      (__log_level_mask & LOG_MASK(LOG_##level))

#define __LOGFB(bet, level, lv, format, ...)        do { \
    if (bet(ALLOW_LOG_LEVEL(level))) { \
        __SYSLOG(level, lv, format, ##__VA_ARGS__); \
    } \
} while (0)

#define __VLOGFB(bet, level, lv, format, ...)       \
    __LOGFB(bet, level, lv, __FILE__ ":%d %s(): " format, __LINE__, __func__, ##__VA_ARGS__)

#define DLOG(format, ...)                           __LOGFB(unlikely, DEBUG, D, format, ##__VA_ARGS__)
#define ILOG(format, ...)                           __LOGF(INFO, I, format, ##__VA_ARGS__)
#define NLOG(format, ...)                           __LOGF(NOTICE, N, format, ##__VA_ARGS__)
#define WLOG(format, ...)                           __VLOGF(WARNING, W, format, ##__VA_ARGS__)
#define ELOG(format, ...)                           __VLOGFB(likely, ERR, E, format, ##__VA_ARGS__)
#define CLOG(format, ...)                           __VLOGFB(likely, CRIT, C, format, ##__VA_ARGS__)

void set_thread_name_for_logger(const char *name);

#if defined(__cplusplus) && __cplusplus >= 201103L /* C++11 */

extern thread_local char __thread_name__[];

#define GET_THREAD_NAME_FOR_LOGGER()                __thread_name__

#else /* Pure C based on POSIX interfaces */

#include <pthread.h>

extern pthread_key_t __thread_name__;

#define GET_THREAD_NAME_FOR_LOGGER()                ((char *)pthread_getspecific(__thread_name__))

#endif /* #if defined(__cplusplus) && __cplusplus >= 201103L */

#define __TLOGF(level, lv, format, ...)             do { \
    __SYSLOG(level, lv, "(%s) " format, GET_THREAD_NAME_FOR_LOGGER(), ##__VA_ARGS__); \
} while (0)

#define __VTLOGF(level, lv, format, ...)            do { \
    __SYSLOG(level, lv, "(%s) " __FILE__ ":%d %s(): " format, GET_THREAD_NAME_FOR_LOGGER(), __LINE__, __func__, ##__VA_ARGS__); \
} while (0)

#define __TLOGFB(bet, level, lv, format, ...)       do { \
    if (bet(ALLOW_LOG_LEVEL(level))) { \
        __SYSLOG(level, lv, "(%s) " format, GET_THREAD_NAME_FOR_LOGGER(), ##__VA_ARGS__); \
    } \
} while (0)

#define __VTLOGFB(bet, level, lv, format, ...)      do { \
    if (bet(ALLOW_LOG_LEVEL(level))) { \
        __SYSLOG(level, lv, "(%s) " __FILE__ ":%d %s(): " format, GET_THREAD_NAME_FOR_LOGGER(), __LINE__, __func__, ##__VA_ARGS__); \
    } \
} while (0)

#define TDLOG(format, ...)                          __TLOGFB(unlikely, DEBUG, D, format, ##__VA_ARGS__)
#define TILOG(format, ...)                          __TLOGF(INFO, I, format, ##__VA_ARGS__)
#define TNLOG(format, ...)                          __TLOGF(NOTICE, N, format, ##__VA_ARGS__)
#define TWLOG(format, ...)                          __VTLOGF(WARNING, W, format, ##__VA_ARGS__)
#define TELOG(format, ...)                          __VTLOGFB(likely, ERR, E, format, ##__VA_ARGS__)
#define TCLOG(format, ...)                          __VTLOGFB(likely, CRIT, C, format, ##__VA_ARGS__)

#endif /* #ifndef __LOGGER_ON_SYSLOG_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-05-25, Man Hung-Coeng:
 *  01. Create.
 */

