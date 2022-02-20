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

#include "socket_supplements.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* sock_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    return strerror(-error_code);
}

int sock_check_status(int fd, int types, int timeout_usecs)
{
    int ret = 0;
    int err_flag = 0;
    socklen_t flag_len = sizeof(err_flag);

    if (types & ~SOCK_STATUS_ABNORMAL)
    {
        struct timeval timev;
        fd_set fd_sets[2];
        fd_set *read_set = ((types & SOCK_STATUS_READABLE) ? &fd_sets[0] : NULL);
        fd_set *write_set = ((types & SOCK_STATUS_WRITABLE) ? &fd_sets[1] : NULL);
        size_t i;

        if (timeout_usecs > 0)
        {
            timev.tv_sec = timeout_usecs / 1000000;
            timev.tv_usec = timeout_usecs % 1000000;
        }

        for (i = 0; i < sizeof(fd_sets)/ sizeof(fd_set); ++i)
        {
            FD_ZERO(&fd_sets[i]);
            FD_SET(fd, &fd_sets[i]);
        }

        if ((ret = select(fd + 1, read_set, write_set, NULL, ((timeout_usecs > 0) ? &timev : NULL))) < 0)
            return -errno;

        ret = 0;
        for (i = 0; i < sizeof(fd_sets)/ sizeof(fd_set); ++i)
        {
            ret |= (FD_ISSET(fd, &fd_sets[i]) ? (1 << i) : 0);
        }
    }

    /*
     * fd is readable or writable or both in some scenarios, say, during connect(),
     * therefore, getsockopt() is called to determine if there is an error occurring on it.
     */
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err_flag, &flag_len) < 0)
        return -errno;

    if (err_flag)
        return -err_flag; /* SOCK_STATUS_ABNORMAL; */

    return ret;
}

int sock_set_nonblocking(int fd)
{
    int options = fcntl(fd, F_GETFL);

    if (options < 0 || fcntl(fd, F_SETFL, options | O_NONBLOCK) < 0)
        return -errno;

    return 0;
}

int sock_connect(int fd, const struct sockaddr *addr, size_t addr_len, int timeout_usecs)
{
    int ret = connect(fd, addr, addr_len);

    if (0 == ret)
        return 0;

    if (EINPROGRESS != errno && EAGAIN != errno)
        return -errno;

    if ((ret = sock_check_status(fd, SOCK_STATUS_WRITABLE, timeout_usecs)) < 0)
        return ret;

    if (!SOCK_IS_WRITABLE(ret))
        return -ETIMEDOUT;

    return 0;
}

size_t sock_send(int fd, const void *buf, size_t len, int flags, int *nullable_error_code)
{
    size_t handled_len = 0;
    int err;
    int *err_ptr = nullable_error_code ? nullable_error_code : &err;

    *err_ptr = 0;

    while (handled_len < len)
    {
        int ret = send(fd, (char *)buf + handled_len, len - handled_len, flags);

        if (ret < 0 && EINTR != errno)
        {
            *err_ptr = -errno;
            break;
        }

        if (ret > 0)
            handled_len += ret;
    }

    return handled_len;
}

size_t sock_recv(int fd, const void *buf, size_t len, int flags, int *nullable_error_code)
{
    size_t handled_len = 0;
    int err;
    int *err_ptr = nullable_error_code ? nullable_error_code : &err;

    *err_ptr = 0;

    while (handled_len < len)
    {
        int ret = recv(fd, (char *)buf + handled_len, len - handled_len, flags);

        if ((0 == ret) || (ret < 0 && EINTR != errno))
        {
            *err_ptr = -errno;
            break;
        }

        if (ret > 0)
            handled_len += ret;
    }

    return handled_len;
}

#ifdef __cplusplus
}
#endif

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-02-20, Man Hung-Coeng:
 *  01. Create.
 */

