/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Formatted logging adapter for simple/flexible projects/libraries
 * written in C or C++ language.
 *
 * Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 */

#ifndef __FORMATTED_LOGGING_ADAPTER_H__
#define __FORMATTED_LOGGING_ADAPTER_H__

#ifndef FMT_LOG

#include <time.h>
#include <stdio.h>

#pragma message("\n\n----\n" \
    "Default FMT_LOG() FMT_LOG_V() are being used,\n" \
    "  both of which output log contents to console and might not be what you want.\n" \
    "To change this, you can specify some macro in command line or your makefile,\n" \
    "  and define your own FMT_LOG*() then.\n" \
    "----\n\n")

/*
 * IMPORTANT Note:
 *
 * Appending a NEWLINE character ("\n") to format string of the last expanded
 * macro statement will flush all contents, including color escape sequences,
 * as a transaction for each logging, since underlying system calls are
 * line-buffering by default. Without this, the following bug will occur:
 *
 * __PRINT_E("This is an error message.\n"); // outputs red message
 * __PRINT_I("This is a normal message.\n"); // the prefix is red, so bad!
 *
 * Note that __PRINT_*() used above are for demonstration only,
 * they're inner macros and strongly discouraged to use in a formal project.
 */

#if defined(__GNUC__) || defined(__clang__) /* ##__VA_ARGS__ is available. */

#define LOG_ADAPTER(_log_func_, _handle_, _tag_, _fmt_, ...)    do { \
    struct timespec __cur_time; \
    struct tm __now; \
\
    clock_gettime(CLOCK_REALTIME, &__cur_time); \
    localtime_r(&__cur_time.tv_sec, &__now); \
\
    _log_func_(_handle_, #_tag_ " %04d-%02d-%02d %02d:%02d:%02d.%09ld " _fmt_ "\n", \
        __now.tm_year + 1900, __now.tm_mon + 1, __now.tm_mday, \
        __now.tm_hour, __now.tm_min, __now.tm_sec, __cur_time.tv_nsec, ##__VA_ARGS__); \
} while (0)

#define LOG_ADAPTER_V(_log_func_, _handle_, _tag_, _fmt_, ...)  do { \
    struct timespec __cur_time; \
    struct tm __now; \
\
    clock_gettime(CLOCK_REALTIME, &__cur_time); \
    localtime_r(&__cur_time.tv_sec, &__now); \
\
    _log_func_(_handle_, #_tag_ " %04d-%02d-%02d %02d:%02d:%02d.%09ld " __FILE__ ":%d %s(): " _fmt_ "\n", \
        __now.tm_year + 1900, __now.tm_mon + 1, __now.tm_mday, \
        __now.tm_hour, __now.tm_min, __now.tm_sec, __cur_time.tv_nsec, __LINE__, __func__, ##__VA_ARGS__); \
} while (0)

#define __PRINT_D(_fmt_, ...)               LOG_ADAPTER(fprintf, stdout, D, _fmt_, ##__VA_ARGS__)
#define __PRINT_D_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stdout, D, _fmt_, ##__VA_ARGS__)

#define __PRINT_I(_fmt_, ...)               LOG_ADAPTER(fprintf, stdout, I, _fmt_, ##__VA_ARGS__)
#define __PRINT_I_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stdout, I, _fmt_, ##__VA_ARGS__)

#define __PRINT_N(_fmt_, ...)               LOG_ADAPTER(fprintf, stdout, N, "\033[0;32m" _fmt_ "\033[0m", ##__VA_ARGS__)
#define __PRINT_N_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stdout, N, "\033[0;32m" _fmt_ "\033[0m", ##__VA_ARGS__)

#define __PRINT_W(_fmt_, ...)               LOG_ADAPTER(fprintf, stderr, W, "\033[0;33m" _fmt_ "\033[0m", ##__VA_ARGS__)
#define __PRINT_W_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stderr, W, "\033[0;33m" _fmt_ "\033[0m", ##__VA_ARGS__)

#define __PRINT_E(_fmt_, ...)               LOG_ADAPTER(fprintf, stderr, E, "\033[0;31m" _fmt_ "\033[0m", ##__VA_ARGS__)
#define __PRINT_E_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stderr, E, "\033[0;31m" _fmt_ "\033[0m", ##__VA_ARGS__)

#define FMT_LOG(_filter_, _tag_, _fmt_, ...)                    do { \
    if (LOG_LEVEL_##_tag_ <= (_filter_)->log_level) \
        __PRINT_##_tag_(_fmt_, ##__VA_ARGS__); \
} while (0)

#define FMT_LOG_V(_filter_, _tag_, _fmt_, ...)                  do { \
    if (LOG_LEVEL_##_tag_ <= (_filter_)->log_level) \
        __PRINT_##_tag_##_V(_fmt_, ##__VA_ARGS__); \
} while (0)

#else /* ##__VA_ARGS__ is NOT available, use __VA_ARGS__ instead. */

#define LOG_ADAPTER(_log_func_, _handle_, _tag_, _fmt_, ...)    do { \
    struct timespec __cur_time; \
    struct tm __now; \
\
    clock_gettime(CLOCK_REALTIME, &__cur_time); \
    localtime_r(&__cur_time.tv_sec, &__now); \
\
    _log_func_(_handle_, #_tag_ " %04d-%02d-%02d %02d:%02d:%02d.%09ld " _fmt_ "\n", \
        __now.tm_year + 1900, __now.tm_mon + 1, __now.tm_mday, \
        __now.tm_hour, __now.tm_min, __now.tm_sec, __cur_time.tv_nsec, __VA_ARGS__); \
} while (0)

#define LOG_ADAPTER_V(_log_func_, _handle_, _tag_, _fmt_, ...)  do { \
    struct timespec __cur_time; \
    struct tm __now; \
\
    clock_gettime(CLOCK_REALTIME, &__cur_time); \
    localtime_r(&__cur_time.tv_sec, &__now); \
\
    _log_func_(_handle_, #_tag_ " %04d-%02d-%02d %02d:%02d:%02d.%09ld " __FILE__ ":%d %s(): " _fmt_ "\n", \
        __now.tm_year + 1900, __now.tm_mon + 1, __now.tm_mday, \
        __now.tm_hour, __now.tm_min, __now.tm_sec, __cur_time.tv_nsec, __LINE__, __func__, __VA_ARGS__); \
} while (0)

#define __PRINT_D(_fmt_, ...)               LOG_ADAPTER(fprintf, stdout, D, _fmt_, __VA_ARGS__)
#define __PRINT_D_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stdout, D, _fmt_, __VA_ARGS__)

#define __PRINT_I(_fmt_, ...)               LOG_ADAPTER(fprintf, stdout, I, _fmt_, __VA_ARGS__)
#define __PRINT_I_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stdout, I, _fmt_, __VA_ARGS__)

#define __PRINT_N(_fmt_, ...)               LOG_ADAPTER(fprintf, stdout, N, "\033[0;32m" _fmt_ "\033[0m", __VA_ARGS__)
#define __PRINT_N_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stdout, N, "\033[0;32m" _fmt_ "\033[0m", __VA_ARGS__)

#define __PRINT_W(_fmt_, ...)               LOG_ADAPTER(fprintf, stderr, W, "\033[0;33m" _fmt_ "\033[0m", __VA_ARGS__)
#define __PRINT_W_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stderr, W, "\033[0;33m" _fmt_ "\033[0m", __VA_ARGS__)

#define __PRINT_E(_fmt_, ...)               LOG_ADAPTER(fprintf, stderr, E, "\033[0;31m" _fmt_ "\033[0m", __VA_ARGS__)
#define __PRINT_E_V(_fmt_, ...)             LOG_ADAPTER_V(fprintf, stderr, E, "\033[0;31m" _fmt_ "\033[0m", __VA_ARGS__)

#define FMT_LOG(_filter_, _tag_, _fmt_, ...)                    do { \
    if (LOG_LEVEL_##_tag_ <= (_filter_)->log_level) \
        __PRINT_##_tag_(_fmt_, __VA_ARGS__); \
} while (0)

#define FMT_LOG_V(_filter_, _tag_, _fmt_, ...)                  do { \
    if (LOG_LEVEL_##_tag_ <= (_filter_)->log_level) \
        __PRINT_##_tag_##_V(_fmt_, __VA_ARGS__); \
} while (0)

#endif /* #if defined(__GNUC__) || defined(__clang__)*/

enum log_level_e
{
    LOG_LEVEL_NONE = 0,

    LOG_LEVEL_ERR = 3, /* same as KERN_ERR used by printk() */
    LOG_LEVEL_E = LOG_LEVEL_ERR,

    LOG_LEVEL_WARNING= 4, /* same as KERN_WARNING used by printk() */
    LOG_LEVEL_W = LOG_LEVEL_WARNING,

    LOG_LEVEL_NOTICE = 5, /* same as KERN_NOTICE used by printk() */
    LOG_LEVEL_N = LOG_LEVEL_NOTICE,

    LOG_LEVEL_INFO = 6, /* same as KERN_INFO used by printk() */
    LOG_LEVEL_I = LOG_LEVEL_INFO,

    LOG_LEVEL_DEBUG = 7, /* same as KERN_DEBUG used by printk() */
    LOG_LEVEL_D = LOG_LEVEL_DEBUG,

    LOG_LEVEL_ALL
};

static enum log_level_e to_log_level(const char *level_str)
{
    if (NULL == level_str || 'I' == level_str[0] || 'i' == level_str[0])
        return LOG_LEVEL_INFO;
    else if ('D' == level_str[0] || 'd' == level_str[0])
        return LOG_LEVEL_DEBUG;
    else if ('N' == level_str[0] || 'n' == level_str[0])
        return LOG_LEVEL_NOTICE;
    else if ('W' == level_str[0] || 'w' == level_str[0])
        return LOG_LEVEL_WARNING;
    else
        return LOG_LEVEL_ERR;
}

#endif /* #ifndef FMT_LOG */

#endif /* #ifndef __FORMATTED_LOGGING_ADAPTER_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-04-04, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 */

