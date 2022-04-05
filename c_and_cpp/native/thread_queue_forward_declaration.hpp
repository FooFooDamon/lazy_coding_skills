/*
 * Forward declaration of class template thread_queue_c,
 * and some definitions of it to make life easier.
 * Including this file instead of thread_queue.hpp in some header files
 * will reduce the overload of header resolving during compilation.
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

#ifndef __THREAD_QUEUE_FORWARD_DECLARATION_HPP__
#define __THREAD_QUEUE_FORWARD_DECLARATION_HPP__

template <typename T, typename seq_container_t>
class thread_queue_c;

#define thread_queue_on_list_c(T)       thread_queue_c<T, std::list<T>>
#define lthread_queue_c(T)              thread_queue_on_list_c(T)
#define lthreaque_c(T)                  lthread_queue_c(T)

#define thread_queue_on_deque_c(T)      thread_queue_c<T, std::deque<T>>
#define dthread_queue_c(T)              thread_queue_on_deque_c(T)
#define dthreaque_c(T)                  dthread_queue_c(T)

#define thread_queue_on_vector_c(T)     thread_queue_c<T, std::vector<T>>
#define vthread_queue_c(T)              thread_queue_on_vector_c(T)
#define vthreaque_c(T)                  vthread_queue_c(T)

#endif /* #ifndef __THREAD_QUEUE_FORWARD_DECLARATION_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-04-05, Man Hung-Coeng:
 *  01. Create.
 */


