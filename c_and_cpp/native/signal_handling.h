/*
 * Signal handling.
 *
 * Copyright (c) 2021-2023 Man Hung-Coeng <udc577@126.com>
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

const char* sig_error(int error_code);

int sig_global_init(void);

void sig_global_reset(void);

int sig_register(int signum, void (*nullable_handler)(int));

int sig_deregister(int signum);

bool sig_has_happened(int signum);

int sig_clear_happen_flag(int signum);

void sig_handler_nop(int signum); /* No OPerations. */

void sig_handler_set_critical_flag(int signum);

bool sig_check_critical_flag(void); /* Works only if the handler above is used. */

const char* sig_number_to_name(int signum);

int sig_name_to_number(const char *signame, size_t name_len);

/*
 * Simple applications can use the function below. For example:
 *
 * int err = sig_simple_register();
 *
 * if (err < 0)
 * {
 *      fprintf(stderr, "sig_simple_register() failed: %s\n", sig_error(err));
 *      return EXIT_FAILURE;
 * }
 *
 * while (1)
 * {
 *      if (sig_check_critical_flag())
 *          break;
 *
 *      // some operations ...
 *
 *      sleep(1); // Should be needed if there're no blocking or waiting operations above.
 * }
 *
 * return EXIT_SUCCESS;
 */
int sig_simple_register(void);

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
 *
 * >>> 2022-01-03, Man Hung-Coeng:
 *  01. Add a function pointer to the parameter list of sig_register()
 *      to support user-defined operations.
 *
 * >>> 2023-12-24, Man Hung-Coeng:
 *  01. Add several functions for simple application usage.
 */


