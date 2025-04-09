/*
 * To-Do and Fix-Me macros.
 *
 * Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
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

#ifndef __TODO_H__
#define __TODO_H__

#ifndef __SRC__
#define __SRC__                 __FILE__
#endif

#ifdef __KERNEL__ /* For Linux kernel and driver development: */

#ifndef TODO
#define TODO(_fmt_, ...)        pr_notice(__SRC__ ":%d %s(): TODO: " _fmt_ "\n", __LINE__, __func__, ##__VA_ARGS__)
#endif

#ifndef FIXME
#define FIXME(_fmt_, ...)       pr_notice(__SRC__ ":%d %s(): FIXME: " _fmt_ "\n", __LINE__, __func__, ##__VA_ARGS__)
#endif

#else /* For user application development below: */

#if (defined(__cplusplus) && __cplusplus >= 201103L) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)

#ifndef LOG_NOTICE
#define LOG_NOTICE(_fmt_, ...)  fprintf(stderr, _fmt_ "\n", ##__VA_ARGS__)
#endif

#ifndef TODO
#define TODO(_fmt_, ...)        LOG_NOTICE(__SRC__ ":%d %s(): TODO: " _fmt_, __LINE__, __func__, ##__VA_ARGS__)
#endif

#ifndef FIXME
#define FIXME(_fmt_, ...)       LOG_NOTICE(__SRC__ ":%d %s(): FIXME: " _fmt_, __LINE__, __func__, ##__VA_ARGS__)
#endif

#else

#ifndef TODO
#define TODO(_fmt_)             fprintf(stderr, __SRC__ ":%d: TODO: " _fmt_, __LINE__)
#endif

#ifndef FIXME
#define FIXME(_fmt_)            fprintf(stderr, __SRC__ ":%d: FIXME: " _fmt_, __LINE__)
#endif

#endif

#endif /* #ifdef __KERNEL__ */

#endif /* #ifndef __TODO_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-04-09, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 */

