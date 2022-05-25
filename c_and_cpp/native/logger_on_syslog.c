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

#include "logger_on_syslog.h"

#include <string.h>
#include <time.h>
#include <syslog.h>

#pragma message("When using logger_on_syslog.h, please include <time.h> and <syslog.h> (if needed).")

int __log_option_flags = 0;
int __log_facility = LOG_USER;
unsigned char __log_level_mask = 0xff;

#define THREAD_NAME_LEN_MAX             15

#if defined(__cplusplus) && __cplusplus >= 201103L /* C++11 */

#pragma message("__thread_name__ is of thread_local type.")

thread_local char __thread_name__[THREAD_NAME_LEN_MAX + 1] = { 0 };

void set_thread_name_for_logger(const char *name)
{
    strncpy(__thread_name__, name, THREAD_NAME_LEN_MAX);
}

#else /* Pure C based on POSIX interfaces */

#include <stdlib.h>

#pragma message("__thread_name__ is of pthread_key_t type.")

pthread_key_t __thread_name__;

/* static const char *S_DEFAULT_THREAD_NAME = "<UNKNOWN_TNAME>"; */

static pthread_once_t s_thread_name_init_once_flag = PTHREAD_ONCE_INIT;

static void free_name_memory(void *name)
{
    if (NULL != name)
        free(name);
}

static void destroy_thread_name_key(void)
{
    pthread_key_delete(__thread_name__);
}

static void init_thread_name_key(void)
{
    if (0 == pthread_key_create(&__thread_name__, free_name_memory))
        atexit(destroy_thread_name_key);
}

void set_thread_name_for_logger(const char *name)
{
    if (pthread_once(&s_thread_name_init_once_flag, init_thread_name_key) >= 0)
    {
        char *thread_name = (char *)pthread_getspecific(__thread_name__);

        if (NULL == thread_name && NULL == (thread_name = (char *)calloc(THREAD_NAME_LEN_MAX + 1, sizeof(char))))
            return;

        strncpy(thread_name, name, THREAD_NAME_LEN_MAX);

        pthread_setspecific(__thread_name__, thread_name);
    }
}

#endif /* #if defined(__cplusplus) && __cplusplus >= 201103L */

#ifdef TEST

#include <stdio.h>
#include <pthread.h>

static void* logging_test(void *arg)
{
    int counter = *(int *)arg;
    char thread_name[THREAD_NAME_LEN_MAX + 1] = { 0 };

    sprintf(thread_name, "syslog_test_%d", counter);

    set_thread_name_for_logger(thread_name);

    DLOG("DLOG of thread[%d]\n", counter);
    ILOG("ILOG of thread[%d]\n", counter);
    NLOG("NLOG of thread[%d]\n", counter);
    WLOG("WLOG of thread[%d]\n", counter);
    ELOG("ELOG of thread[%d]\n", counter);
    CLOG("CLOG of thread[%d]\n", counter);

    TDLOG("TDLOG of thread[%d]", counter);
    TILOG("TILOG of thread[%d]", counter);
    TNLOG("TNLOG of thread[%d]", counter);
    TWLOG("TWLOG of thread[%d]", counter);
    TELOG("TELOG of thread[%d]", counter);
    TCLOG("TCLOG of thread[%d]", counter);

    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t ids[5];
    size_t i;

    OPEN_SYSLOG("syslog_client", LOG_CONS | LOG_NDELAY, LOG_USER, LOG_UPTO(LOG_INFO));

    for (i = 0; i < sizeof(ids) / sizeof(pthread_t); ++i)
    {
        pthread_create(&ids[i], NULL, logging_test, &i);
    }

    for (i = 0; i < sizeof(ids) / sizeof(pthread_t); ++i)
    {
        pthread_join(ids[i], NULL);
    }

    CLOSE_SYSLOG();

    printf("Test finished, please open /var/log/syslog (or something like that) to check the result.\n");

    return 0;
}

#endif /* #ifdef TEST */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-05-25, Man Hung-Coeng:
 *  01. Create.
 */

