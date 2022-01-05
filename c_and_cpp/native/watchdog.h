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

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: May be void* or something on other platforms in future, if supported. */
typedef int watchdog_t;

const char* watchdog_error(int error_code);

watchdog_t watchdog_open(const char *device);

int watchdog_close(watchdog_t dev);

int watchdog_enable(watchdog_t dev);

int watchdog_disable(watchdog_t dev);

int watchdog_get_timeout(watchdog_t dev);

int watchdog_set_timeout(watchdog_t dev, int timeout_secs);

int watchdog_feed(watchdog_t dev);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __WATCHDOG_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-01-05, Man Hung-Coeng:
 *  01. Create.
 */


