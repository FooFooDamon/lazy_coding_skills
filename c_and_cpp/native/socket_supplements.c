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

enum
{
    SOCK_ERR_UNKNOWN = 1
    , SOCK_ERR_NOT_SUPPORTED
    , SOCK_ERR_NOT_IMPLEMENTED
    , SOCK_ERR_SELF_CONNECTED

    /* NOTE: All error codes should be defined ahead of this. */
    , SOCK_ERR_END
};

static const char* const S_ERRORS[] = {
    "Unknown error"
    , "Not supported"
    , "Not implemented"
    , "Self connected"
};

const char* sock_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    if (error_code <= -SOCK_ERR_END)
        return strerror(-error_code - SOCK_ERR_END);

    return S_ERRORS[-error_code - 1];
}

int sock_create(int domain, int type, int protocol, bool is_nonblocking)
{
#if defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__)

    int fd = socket(domain, (is_nonblocking ? (type | SOCK_NONBLOCK) : type), protocol);

    return (fd >= 0) ? fd : -(errno + SOCK_ERR_END);

#else

    int ret = socket(domain, type, protocol);
    int fd = ret;

    if (fd < 0)
        return -(errno + SOCK_ERR_END);

    if (is_nonblocking && (ret = sock_set_nonblocking(fd)) < 0)
    {
        close(fd);
        return ret;
    }

    return fd;

#endif
}

int sock_destroy(int fd)
{
    return (close(fd) < 0) ? -(errno + SOCK_ERR_END) : 0;
}

int sock_set_nonblocking(int fd)
{
    int options = fcntl(fd, F_GETFL);

    if (options < 0 || fcntl(fd, F_SETFL, options | O_NONBLOCK) < 0)
        return -(errno + SOCK_ERR_END);

    return 0;
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

        for (i = 0; i < sizeof(fd_sets) / sizeof(fd_set); ++i)
        {
            FD_ZERO(&fd_sets[i]);
            FD_SET(fd, &fd_sets[i]);
        }

        if (select(fd + 1, read_set, write_set, NULL, ((timeout_usecs > 0) ? &timev : NULL)) < 0)
            return -(errno + SOCK_ERR_END);

        ret = 0;
        for (i = 0; i < sizeof(fd_sets) / sizeof(fd_set); ++i)
        {
            ret |= (FD_ISSET(fd, &fd_sets[i]) ? (1 << i) : 0);
        }
    }

    /*
     * fd is readable or writable or both in some scenarios, say, during connect(),
     * therefore, getsockopt() is called to determine if there is an error occurring on it.
     */
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err_flag, &flag_len) < 0)
        return -(errno + SOCK_ERR_END);

    if (err_flag)
        return -err_flag; /* SOCK_STATUS_ABNORMAL; */

    return ret;
}

int sock_bind(int fd, bool allow_addr_reuse, const struct sockaddr *addr, size_t addr_len)
{
    if (allow_addr_reuse)
    {
        const int REUSE_FLAG = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&REUSE_FLAG, sizeof(REUSE_FLAG)) < 0)
            return -(errno + SOCK_ERR_END);
    }

    if (bind(fd, addr, addr_len) < 0)
        return -(errno + SOCK_ERR_END);

    return 0;
}

int sock_listen(int fd, int backlog)
{
    return (listen(fd, backlog) < 0) ? -(errno + SOCK_ERR_END) : 0;
}

int sock_accept(int fd, bool is_nonblocking, bool allow_self_connection, struct sockaddr *addr, socklen_ptr_t addr_len)
{
    int ret = accept(fd, addr, (socklen_t *)addr_len);
    int client_fd = ret;

    if (client_fd < 0)
        return -(errno + SOCK_ERR_END);

    if (is_nonblocking && (ret = sock_set_nonblocking(client_fd)) < 0)
    {
        close(client_fd);
        return ret;
    }

    if (!allow_self_connection)
    {
        struct sockaddr self_addr;
        socklen_t self_addr_len = sizeof(struct sockaddr);

        if (getsockname(fd, &self_addr, &self_addr_len) < 0)
        {
            close(client_fd);
            return -(errno + SOCK_ERR_END);
        }

        if (self_addr_len == *(socklen_t *)addr_len && 0 == memcmp(&self_addr, addr, self_addr_len))
        {
            close(client_fd);
            return -SOCK_ERR_SELF_CONNECTED;
        }
    }

    return client_fd;
}

int sock_connect(int fd, const struct sockaddr *addr, size_t addr_len, int timeout_usecs)
{
    int ret = connect(fd, addr, addr_len);

    if (0 == ret)
        return 0;

    if (EINPROGRESS != errno && EAGAIN != errno)
        return -(errno + SOCK_ERR_END);

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
            *err_ptr = -(errno + SOCK_ERR_END);
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
            *err_ptr = -(errno + SOCK_ERR_END);
            break;
        }

        if (ret > 0)
            handled_len += ret;
    }

    return handled_len;
}

#ifdef TEST

#include <stdio.h>

int main(int argc, char **argv)
{
    printf("TODO: ...\n");
    return 0;
}

#endif /* #ifdef TEST */

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
 *
 * >>> 2022-02-21, Man Hung-Coeng:
 *  01. Add sock_create(), sock_destroy(), sock_bind(), sock_listen()
 *      and sock_accept().
 */

