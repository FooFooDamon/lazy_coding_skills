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

#ifndef __SLEEPS_H__
#define __SLEEPS_H__

#ifdef __cplusplus
extern "C" {
#endif

const char* sleep_error(int error_code);

int sleep_seconds(int seconds);
int sleep_seconds_fully(int seconds);

int sleep_milliseconds(int milliseconds);
int sleep_milliseconds_fully(int milliseconds);

int sleep_microseconds(int microseconds);
int sleep_microseconds_fully(int microseconds);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __SLEEPS_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-01-07, Man Hung-Coeng:
 *  01. Create.
 */


