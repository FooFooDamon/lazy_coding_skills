/*
 * Watchdog operations.
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

#include "watchdog.h"

#include <errno.h>
#include <string.h>
#if defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__)
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    WDOG_ERR_UNKNOWN = 1
    , WDOG_ERR_NOT_SUPPORTED
    , WDOG_ERR_NOT_IMPLEMENTED

    , WDOG_ERR_END /* NOTE: All error codes should be defined ahead of this. */
};

static const char* const S_ERRORS[] = {
    "Unknown error"
    , "Not supported"
    , "Not implemented"
};

const char* watchdog_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    if (error_code < -WDOG_ERR_END)
        return strerror(-error_code - WDOG_ERR_END);

    return S_ERRORS[-error_code - 1];
}

#if defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__)

watchdog_t watchdog_open(const char *device)
{
    watchdog_t dev = open(device, O_RDWR);

    return (dev >= 0) ? dev : -(errno + WDOG_ERR_END);
}

int watchdog_close(watchdog_t dev)
{
    int ret = close(dev);

    return (ret >= 0) ? 0 : -(errno + WDOG_ERR_END);
}

static int __xable_watchdog(watchdog_t dev, int cmd)
{
    int ret = ioctl(dev, WDIOC_SETOPTIONS, &cmd);

    return (ret >= 0) ? ret : -(errno + WDOG_ERR_END);
}

int watchdog_enable(watchdog_t dev)
{
    return __xable_watchdog(dev, WDIOS_ENABLECARD);
}

int watchdog_disable(watchdog_t dev)
{
    return __xable_watchdog(dev, WDIOS_DISABLECARD);
}

int watchdog_get_timeout(watchdog_t dev)
{
    int timeout_secs = 0;
    int ret = ioctl(dev, WDIOC_GETTIMEOUT, &timeout_secs);

    return (ret >= 0) ? timeout_secs : -(errno + WDOG_ERR_END);
}

int watchdog_set_timeout(watchdog_t dev, int timeout_secs)
{
    int ret = ioctl(dev, WDIOC_SETTIMEOUT, &timeout_secs);

    return (ret >= 0) ? timeout_secs : -(errno + WDOG_ERR_END);
}

int watchdog_feed(watchdog_t dev)
{
    int ret = ioctl(dev, WDIOC_KEEPALIVE, 0);

    return (ret >= 0) ? ret : -(errno + WDOG_ERR_END);
}

#else /* Not Linux */

watchdog_t watchdog_open(const char *device)
{
    return -WDOG_ERR_NOT_SUPPORTED;
}

int watchdog_close(watchdog_t dev)
{
    return -WDOG_ERR_NOT_SUPPORTED;
}

int watchdog_enable(watchdog_t dev)
{
    return -WDOG_ERR_NOT_SUPPORTED;
}

int watchdog_disable(watchdog_t dev)
{
    return -WDOG_ERR_NOT_SUPPORTED;
}

int watchdog_get_timeout(watchdog_t dev)
{
    return -WDOG_ERR_NOT_SUPPORTED;
}

int watchdog_set_timeout(watchdog_t dev, int timeout_secs)
{
    return -WDOG_ERR_NOT_SUPPORTED;
}

int watchdog_feed(watchdog_t dev)
{
    return -WDOG_ERR_NOT_SUPPORTED;
}

#endif /* #if defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) */

#ifdef TEST

#include <stdio.h>

int main(int argc, char **argv)
{
    printf("TODO ...");

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
 * >>> 2022-01-05, Man Hung-Coeng:
 *  01. Create.
 */

