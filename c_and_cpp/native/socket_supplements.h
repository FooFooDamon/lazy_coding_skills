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

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr;

enum
{
    SOCK_STATUS_READABLE = 1 << 0
    , SOCK_STATUS_WRITABLE = 1 << 1
    , SOCK_STATUS_ABNORMAL = 1 << 2
};

#define SOCK_IS_READABLE(status_bits)               (status_bits & SOCK_STATUS_READABLE)
#define SOCK_IS_WRITABLE(status_bits)               (status_bits & SOCK_STATUS_WRITABLE)

const char* sock_error(int error_code);

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

int sock_set_nonblocking(int fd);

int sock_connect(int fd, const struct sockaddr *addr, size_t addr_len, int timeout_usecs);

/* NOTE: It's recommended to sock_set_nonblocking() or/and setsockopt(SO_SNDTIMEO) before this function. */
size_t sock_send(int fd, const void *buf, size_t len, int flags, int *nullable_error_code);

/* NOTE: It's recommended to sock_set_nonblocking() or/and setsockopt(SO_RCVTIMEO) before this function. */
size_t sock_recv(int fd, const void *buf, size_t len, int flags, int *nullable_error_code);

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
 */

