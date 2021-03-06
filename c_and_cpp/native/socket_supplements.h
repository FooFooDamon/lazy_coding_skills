/*
 * Supplements to socket operation.
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

#ifndef __SOCKET_SUPPLEMENTS_H__
#define __SOCKET_SUPPLEMENTS_H__

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr;
typedef void* socklen_ptr_t; /* The argument type should be "socklen_t *". */

enum
{
    SOCK_STATUS_READABLE = 1 << 0
    , SOCK_STATUS_WRITABLE = 1 << 1
    , SOCK_STATUS_ABNORMAL = 1 << 2
};

#define SOCK_IS_READABLE(status_bits)               (status_bits & SOCK_STATUS_READABLE)
#define SOCK_IS_WRITABLE(status_bits)               (status_bits & SOCK_STATUS_WRITABLE)

#pragma message("To use errno and its candidate values, <errno.h> or something similar should be included or defined manually.")

#ifndef SOCK_IS_DISCONNECTED
#define SOCK_IS_DISCONNECTED(standard_errno)        (EPIPE == (standard_errno) || ECONNRESET == (standard_errno) \
                                                        || ENOTCONN == (standard_errno) || ESHUTDOWN == (standard_errno))
#endif
#ifndef SOCK_IS_OFFLINE
#define SOCK_IS_OFFLINE(standard_errno)             (ECONNREFUSED == (standard_errno))
#endif
#ifndef SOCK_CONNECTION_IS_LOST
#define SOCK_CONNECTION_IS_LOST(standard_errno)     SOCK_IS_DISCONNECTED(standard_errno) || SOCK_IS_OFFLINE(standard_errno)
#endif
#ifndef SOCK_SHOULD_TRY_LATER
#define SOCK_SHOULD_TRY_LATER(standard_errno)       (EAGAIN == (standard_errno) || EWOULDBLOCK == (standard_errno))
#endif

const char* sock_error(int error_code);

int sock_create(int domain, int type, int protocol, bool is_nonblocking);

int sock_destroy(int fd);

int sock_set_nonblocking(int fd);

/*
 * Usage example 1:
 *      int types = SOCK_STATUS_READABLE | SOCK_STATUS_WRITABLE; // SOCK_STATUS_ABNORMAL is optional.
 *      int ret = sock_check_status(fd, types, timeout);
 *      if (ret < 0)
 *          fprintf(stderr, "*** sock_check_status() failed or socket(%d) is abnormal: %s\n", fd, sock_error(ret));
 *      else
 *      {
 *          if (SOCK_IS_READABLE(ret))
 *              printf("Socket(%d) is readable.\n", fd);
 *
 *          if (SOCK_IS_WRITABLE(ret))
 *              printf("Socket(%d) is writable.\n", fd);
 *      }
 *
 * Usage example 2:
 *      int ret = sock_check_status(fd, SOCK_STATUS_ABNORMAL, timeout);
 *      if (ret < 0)
 *          fprintf(stderr, "*** sock_check_status() failed or socket(%d) is abnormal: %s\n", fd, sock_error(ret));
 *      else
 *          printf("Socket(%d) is normal.\n", fd);
 */
int sock_check_status(int fd, int types, int timeout_usecs);

int sock_bind(int fd, bool allow_addr_reuse, const struct sockaddr *addr, size_t addr_len);

int sock_listen(int fd, int backlog);

int sock_accept(int fd, bool is_nonblocking, bool allow_self_connection, struct sockaddr *addr, socklen_ptr_t addr_len);

int sock_connect(int fd, const struct sockaddr *addr, size_t addr_len, int timeout_usecs);

/* NOTE: It's recommended to sock_set_nonblocking() and select()/poll()/epoll() before this function. */
size_t sock_send(int fd, const void *buf, size_t len, int flags, int *nullable_standard_errno);

/* NOTE: It's recommended to sock_set_nonblocking() and select()/poll()/epoll() before this function. */
/* EXTRA: For a connection-oriented socket, return value of 0 and errno of EPIPE means (orderly) shutdown. */
size_t sock_recv(int fd, const void *buf, size_t len, int flags, int *nullable_standard_errno);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __SOCKET_SUPPLEMENTS_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-02-20, Man Hung-Coeng:
 *  01. Create.
 *
 * >>> 2022-02-21, Man Hung-Coeng:
 *  01. Add sock_create(), sock_destroy(), sock_bind(), sock_listen()
 *      and sock_accept().
 *
 * >>> 2022-03-27, Man Hung-Coeng:
 *  01. Rename the parameter nullable_error_code of sock_send() and sock_recv()
 *      to nullable_standard_errno, and change its meaning and value as well.
 *
 * >>> 2022-03-28, Man Hung-Coeng:
 *  01. Add several macros: SOCK_IS_DISCONNECTED(), SOCK_IS_OFFLINE(),
 *      SOCK_CONNECTION_IS_LOST() and SOCK_SHOULD_TRY_LATER().
 */

