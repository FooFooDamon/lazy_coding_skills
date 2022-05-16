/*
 * A function with No OPeration, which is usually used
 * for eliminating some stupid warnings,
 * e.g., cppcheck[unreadVariable] warning on members of an union-type variable,
 * without any runtime overhead (in non-debug mode).
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

#ifndef __NOP_H__
#define __NOP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
inline void nop(const char *format, ...){}
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
static inline void nop(const char *format, ...){}
#else
#error The keyword "inline" is available in C++, or C99 or above!
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __NOP_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-05-16, Man Hung-Coeng:
 *  01. Create.
 */

