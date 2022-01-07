/*
 * Interruptable/Uninterruptable sleep functions with different kinds of units.
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

#include "sleeps.h"

#include <errno.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* sleep_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    return strerror(-error_code);
}

int sleep_seconds(int seconds)
{
#if 0
    /* NOTE: error: initializer element is not computable at load time. Same in other functions. */
    struct timespec request = { seconds, 0 };
#else
    struct timespec request = { 0 };
#endif
    struct timespec remain = { 0 };
    int return_value = -1;

    request.tv_sec = seconds;

    return_value = nanosleep(&request, &remain);

    return (0 == return_value) ? 0 : ((EINTR == errno && remain.tv_sec > 0) ? remain.tv_sec : -errno);
}

int sleep_seconds_fully(int seconds)
{
    struct timespec request = { 0 };
    struct timespec remain = { 0 };
    int return_value = -1;

    request.tv_sec = seconds;

    do
    {
        return_value = nanosleep(&request, &remain);
    }
    while (EINTR == errno && (remain.tv_sec > 0 || remain.tv_nsec > 0));

    return (0 == return_value) ? 0 : -errno;
}

int sleep_milliseconds(int milliseconds)
{
    struct timespec request = { 0 };
    struct timespec remain = { 0 };
    int return_value = -1;

    request.tv_sec = milliseconds / 1000;
    request.tv_nsec = (milliseconds % 1000) * 1000000;

    return_value = nanosleep(&request, &remain);

    return (0 == return_value) ? 0 : ((EINTR == errno) ? (remain.tv_sec * 1000 + remain.tv_nsec / 1000000) : -errno);
}

int sleep_milliseconds_fully(int milliseconds)
{
    struct timespec request = { 0 };
    struct timespec remain = { 0 };
    int return_value = -1;

    request.tv_sec = milliseconds / 1000;
    request.tv_nsec = (milliseconds % 1000) * 1000000;

    do
    {
        return_value = nanosleep(&request, &remain);
    }
    while (EINTR == errno && (remain.tv_sec > 0 || remain.tv_nsec > 0));

    return (0 == return_value) ? 0 : -errno;
}

int sleep_microseconds(int microseconds)
{
    struct timespec request = { 0 };
    struct timespec remain = { 0 };
    int return_value = -1;

    request.tv_sec = microseconds / 1000000;
    request.tv_nsec = (microseconds % 1000000) * 1000;

    return_value = nanosleep(&request, &remain);

    return (0 == return_value) ? 0 : ((EINTR == errno) ? (remain.tv_sec * 1000000 + remain.tv_nsec / 1000) : -errno);
}

int sleep_microseconds_fully(int microseconds)
{
    struct timespec request = { 0 };
    struct timespec remain = { 0 };
    int return_value = -1;

    request.tv_sec = microseconds / 1000000;
    request.tv_nsec = (microseconds % 1000000) * 1000;

    do
    {
        return_value = nanosleep(&request, &remain);
    }
    while (EINTR == errno && (remain.tv_sec > 0 || remain.tv_nsec > 0));

    return (0 == return_value) ? 0 : -errno;
}

#ifdef TEST

#include <stdio.h>

int main(int argc, char **argv)
{
    printf("TODO ...\n");

    return 0;
}

#endif /* #ifdef TEST */

#ifdef __cplusplus
}
#endif

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-01-07, Man Hung-Coeng:
 *  01. Create.
 */

