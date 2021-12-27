/*
 * Signal handling.
 *
 * Copyright (c) 2021 Man Hung-Coeng <udc577@126.com>
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

#ifndef __SIGNAL_HANDLING_H__
#define __SIGNAL_HANDLING_H__

#include <stdbool.h> /* For bool, true and false. */
#include <stddef.h> /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/*#define true    1
#define false   0
typedef char    bool;*/

const char* sig_error(int error_code);

int sig_global_init(void);

void sig_global_reset(void);

int sig_register(int signum);

int sig_deregister(int signum);

bool sig_has_happened(int signum);

int sig_clear_happen_flag(int signum);

const char* sig_number_to_name(int signum);

int sig_name_to_number(const char *signame, size_t name_len);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __SIGNAL_HANDLING_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2021-12-25, Man Hung-Coeng:
 *  01. Create.
 */

