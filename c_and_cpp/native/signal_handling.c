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

#include "signal_handling.h"

#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    SIG_ERR_UNKNOWN = 1
    , SIG_ERR_NOT_IMPLEMENTED
    , SIG_ERR_MEM_ALLOC
    , SIG_ERR_ZERO_LENGTH
    , SIG_ERR_STRING_TOO_LONG
    , SIG_ERR_NOT_INITIALIZED
    , SIG_ERR_INVALID_SIGNAL_NUM
    , SIG_ERR_INVALID_SIGNAL_NAME

    , SIG_ERR_END /* NOTE: All error codes should be defined ahead of this. */
};

static const char* const S_ERRORS[] = {
    "Unknown error"
    , "Not implemented"
    , "Failed to allocate memory"
    , "Zero length"
    , "String too long"
    , "Not initialized"
    , "Invalid signal number"
    , "Invalid signal name"
};

const char* sig_error(int error_code)
{
    if (error_code >= 0)
        return "OK";

    if (error_code <= -SIG_ERR_END)
        return strerror(-error_code - SIG_ERR_END);

    return S_ERRORS[-error_code - 1];
}

#ifndef SIG_NAME_LEN_MAX
#define SIG_NAME_LEN_MAX            15
#endif

#ifndef SIG_NUM_START
#define SIG_NUM_START               1
#endif

#ifndef SIG_NUM_END
#define SIG_NUM_END                 64
#endif

#ifndef SIG_INVALID_NUM
#define SIG_INVALID_NUM             -1
#endif

typedef struct sig_info
{
    int num;
    char name[SIG_NAME_LEN_MAX + 1];
    volatile sig_atomic_t happened;
    void (*handler)(int);
} sig_info_t;

static sig_info_t *s_sig_info_tables = NULL;

#define SIG_INFO_READY()            (NULL != s_sig_info_tables)
#define SIG_INFO_AT(num)            s_sig_info_tables[num - SIG_NUM_START]

int sig_global_init(void)
{
#if defined(__unix) || defined(__unix__) || defined(unix) \
    || defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) \
    || defined(__APPLE__) || defined(__MACH__) || defined(macintosh)
    FILE *stream = NULL;
    char buf[(SIG_NAME_LEN_MAX + 1) * 2] = { 0 };
    char cmd[128] = { 0 };
    const size_t MAX_SIG_COUNT = SIG_NUM_END - SIG_NUM_START + 1;
#else /* For example: Windows. */
    const sig_info_t MIN_INFO_TABLES[] = {
        { SIGABRT, "ABRT", 0, NULL }
        , { SIGFPE, "FPE", 0, NULL }
        , { SIGILL, "ILL", 0, NULL }
        , { SIGINT, "INT", 0, NULL }
        , { SIGSEGV, "SEGV", 0, NULL }
        , { SIGTERM, "TERM", 0, NULL }
    };
    const size_t MAX_SIG_COUNT = sizeof(MIN_INFO_TABLES) / sizeof(sig_info_t);
    size_t i = 0;
#endif /* A bunch of Unix-like system macros. */

    if (SIG_INFO_READY())
        return 0;

    if (NULL == (s_sig_info_tables = (sig_info_t *)calloc(SIG_NUM_END - SIG_NUM_START + 1, sizeof(sig_info_t))))
        return -SIG_ERR_MEM_ALLOC;

    atexit(sig_global_reset);


#if defined(__unix) || defined(__unix__) || defined(unix) \
    || defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) \
    || defined(__APPLE__) || defined(__MACH__) || defined(macintosh)

/*
 * If "kill -l" is executed in a real terminal, use one of the two commaneds below.
 * Note that the one using awk to do string replacement is more compatible and works well on OS X
 * while the other may not!
#define SIG_GET_NUMS_AND_NAMES_CMD  "kill -l | awk '{ gsub(\"\\t\", \"\\n\", $0); gsub(\"SIG\", \"\", $0); print; }'"
#define SIG_GET_NUMS_AND_NAMES_CMD  "kill -l | sed 's/\\t/\\n/g' | sed -e 's/SIG//'"
*/
/* If "kill -l" is executed within a program, use this. */
#define SIG_GET_NUMS_AND_NAMES_CMD  "for i in `seq %d %d`; do printf '%%d %%s\\n' $i `kill -l $i 2> /dev/null`; done"

#ifdef __STRICT_ANSI__
#pragma message("Oops, popen() is not available in ANSI C!")
    sprintf(cmd, SIG_GET_NUMS_AND_NAMES_CMD" > .signal_handling_init.tmp", SIG_NUM_START, SIG_NUM_END);
    system(cmd);
    stream = fopen(".signal_handling_init.tmp", "r");
#else
    sprintf(cmd, SIG_GET_NUMS_AND_NAMES_CMD, SIG_NUM_START, SIG_NUM_END);
    stream = popen(cmd, "r");
#endif
    if (NULL == stream)
        return -(errno + SIG_ERR_END);

    while (NULL != fgets(buf, sizeof(buf), stream))
    {
        char *space_ptr = strchr(buf, ' ');
        int signum = (NULL == space_ptr) ? SIG_INVALID_NUM : atoi(buf);

        if (NULL != space_ptr && signum >= SIG_NUM_START && signum <= SIG_NUM_END)
        {
            SIG_INFO_AT(signum).num = signum;
            strncpy(SIG_INFO_AT(signum).name, space_ptr + 1, SIG_NAME_LEN_MAX);
            /* NOTE: Eliminate the trailing newline character. */
            *strchr(SIG_INFO_AT(signum).name, '\n') = '\0';
        }
    }
#ifdef __STRICT_ANSI__
    fclose(stream);
    system("rm -f .signal_handling_init.tmp");
#else
    pclose(stream);
#endif

#else /* For example: Windows. */

    for (; i < MAX_SIG_COUNT; ++i)
    {
        const sig_info_t *info = &MIN_INFO_TABLES[i];

        SIG_INFO_AT(i).num = info->num;
        strcpy(SIG_INFO_AT(i).name, info->name);
    }

#endif /* A bunch of Unix-like system macros. */

    return MAX_SIG_COUNT;
}

void sig_global_reset(void)
{
    if (NULL != s_sig_info_tables)
    {
        free(s_sig_info_tables);
        s_sig_info_tables = NULL;
    }
}

static void sig_set_happen_flag_and_call_handler(int signum)
{
    SIG_INFO_AT(signum).happened = 1;
    if (NULL != SIG_INFO_AT(signum).handler)
        SIG_INFO_AT(signum).handler(signum);
}

int sig_register(int signum, void (*nullable_handler)(int))
{
#if (defined(__unix) || defined(__unix__) || defined(unix) \
    || defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) \
    || defined(__APPLE__) || defined(__MACH__) || defined(macintosh)) \
    && !defined(__STRICT_ANSI__)
    struct sigaction act;
#endif

    if (!SIG_INFO_READY())
        return -SIG_ERR_NOT_INITIALIZED;

    if (signum < SIG_NUM_START || signum > SIG_NUM_END || SIG_INVALID_NUM == SIG_INFO_AT(signum).num)
        return -SIG_ERR_INVALID_SIGNAL_NUM;

    /* Use signal(SIGxxx, SIG_DFL) or signal(SIGxxx, SIG_IGN) instead. */
    if (SIG_DFL == nullable_handler || SIG_IGN == nullable_handler)
        return -SIG_ERR_NOT_IMPLEMENTED;

    SIG_INFO_AT(signum).handler = nullable_handler;

#if (defined(__unix) || defined(__unix__) || defined(unix) \
    || defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) \
    || defined(__APPLE__) || defined(__MACH__) || defined(macintosh)) \
    && !defined(__STRICT_ANSI__)
/*#pragma message("sig_register(): Using new sigaction().")*/
    act.sa_handler = sig_set_happen_flag_and_call_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#if defined(SA_INTERRUPT) || defined(SA_RESTART)
    act.sa_flags |= SA_RESTART;
#endif
    if (sigaction(signum, &act, NULL) < 0)
#else
#pragma message("sig_register(): Using old signal().")
    if (SIG_ERR == signal(signum, sig_set_happen_flag_and_call_handler))
#endif /* A bunch of Unix-like system macros and __STRICT_ANSI__. */
        return -(errno + SIG_ERR_END);

    return 0;
}

int sig_deregister(int signum)
{
    if (!SIG_INFO_READY())
        return -SIG_ERR_NOT_INITIALIZED;

    if (signum < SIG_NUM_START || signum > SIG_NUM_END || SIG_INVALID_NUM == SIG_INFO_AT(signum).num)
        return -SIG_ERR_INVALID_SIGNAL_NUM;

    if (SIG_ERR == signal(signum, SIG_DFL))
        return -(errno + SIG_ERR_END);

    return 0;
}

bool sig_has_happened(int signum)
{
    if (!SIG_INFO_READY())
        return false;

    if (signum < SIG_NUM_START || signum > SIG_NUM_END || SIG_INVALID_NUM == SIG_INFO_AT(signum).num)
        return false;

    return SIG_INFO_AT(signum).happened;
}

int sig_clear_happen_flag(int signum)
{
    if (!SIG_INFO_READY())
        return -SIG_ERR_NOT_INITIALIZED;

    if (signum < SIG_NUM_START || signum > SIG_NUM_END || SIG_INVALID_NUM == SIG_INFO_AT(signum).num)
        return -SIG_ERR_INVALID_SIGNAL_NUM;

    SIG_INFO_AT(signum).happened = 0;

    return 0;
}

void sig_handler_nop(int signum)
{
    /* No OPerations. */
}

static volatile bool s_critical_flag = false;

void sig_handler_set_critical_flag(int signum)
{
    s_critical_flag = true;
}

bool sig_check_critical_flag(void)
{
    return s_critical_flag;
}

const char* sig_number_to_name(int signum)
{
    if (!SIG_INFO_READY())
        return NULL;

    if (signum < SIG_NUM_START || signum > SIG_NUM_END || SIG_INVALID_NUM == SIG_INFO_AT(signum).num)
        return NULL;

    return SIG_INFO_AT(signum).name;
}

int sig_name_to_number(const char *signame, size_t name_len)
{
    int i = SIG_NUM_START;

    if (name_len > SIG_NAME_LEN_MAX)
        return -SIG_ERR_STRING_TOO_LONG;

    if (0 == name_len)
        return -SIG_ERR_ZERO_LENGTH;

    if (!SIG_INFO_READY())
        return -SIG_ERR_NOT_INITIALIZED;

    for (; i <= SIG_NUM_END; ++i)
    {
        if (SIG_INVALID_NUM != SIG_INFO_AT(i).num && 0 == strncmp(signame, SIG_INFO_AT(i).name, SIG_NAME_LEN_MAX))
            return i;
    }

    return -SIG_ERR_INVALID_SIGNAL_NAME;
}

int sig_simple_register(void)
{
    size_t i;
    const int CRITICAL_SIGNALS[] = {
        SIGINT, SIGABRT, SIGTERM
    };
    int err = sig_global_init();

    if (err < 0)
        return err;

    for (i = 0; i < sizeof(CRITICAL_SIGNALS) / sizeof(CRITICAL_SIGNALS[0]); ++i)
    {
        if ((err = sig_register(CRITICAL_SIGNALS[i], sig_handler_set_critical_flag)) < 0)
            break;
    }

    if (err < 0)
    {
        for (i = 0; i < sizeof(CRITICAL_SIGNALS) / sizeof(CRITICAL_SIGNALS[0]); ++i)
        {
            signal(CRITICAL_SIGNALS[i], SIG_DFL);
        }
    }
#if !defined(WIN32) && !defined(_WIN32)
    else
    {
        signal(SIGPIPE, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
    }
#endif

    return err;
}

#ifdef TEST

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#define sleep(secs)             Sleep(secs * 1000)
#else
#include <unistd.h>
#endif

static bool s_should_exit = false;

static void set_exit_flag(int signum)
{
    if (SIGINT == signum)
        s_should_exit = true;
}

static bool should_exit(void)
{
    return s_should_exit;
}

int main(int argc, char **argv)
{
    int i = 0;
    int err = sig_global_init();

    if (err < 0)
    {
        fprintf(stderr, "sig_global_init() failed: %s\n", sig_error(err));

        return EXIT_FAILURE;
    }

    for (i = SIG_NUM_START - 1; i <= SIG_NUM_END + 1; ++i)
    {
        const char *signame = sig_number_to_name(i);
        int signum = (NULL == signame) ? SIG_INVALID_NUM : sig_name_to_number(signame, strlen(signame));

        err = sig_register(i, (SIGINT == signum) ? set_exit_flag : sig_handler_nop);

        fprintf((err < 0) ? stderr : stdout, "sig_register() for [%d -> %s -> %d]: %s\n",
            i, ((NULL == signame) ? "<UNKNOWN>" : signame), signum, sig_error(err));
    }

    while(1)
    {
        for (i = SIG_NUM_START; i < SIG_NUM_END / 2; ++i) /* NOTE: Some undefined signals will kill the process! */
        {
#if defined(__unix) || defined(__unix__) || defined(unix) \
    || defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) \
    || defined(__APPLE__) || defined(__MACH__) || defined(macintosh)
            if (SIGKILL == i || SIGSTOP == i)
                continue;
#endif
            if (SIGINT != i)
            {
                raise(i);
                printf("Raised: %d|%s\n", i, sig_number_to_name(i));
            }
        }

        for (i = SIG_NUM_START - 1; i <= SIG_NUM_END + 1; ++i)
        {
            if (sig_has_happened(i))
            {
                printf("Received: %d|%s\n", i, sig_number_to_name(i));
                sig_clear_happen_flag(i);
            }
        }

        if (should_exit())
            break;

        printf("-------- Press Ctrl+C if you want to finish. --------\n");

        sleep(3);
    }

    for (i = SIG_NUM_START - 1; i <= SIG_NUM_END + 1; ++i)
    {
        const char *signame = sig_number_to_name(i);

        err = sig_deregister(i);

        fprintf((err < 0) ? stderr : stdout, "sig_deregister() for [%d -> %s]: %s\n",
            i, signame, sig_error(err));
    }

    return EXIT_SUCCESS;
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
 * >>> 2021-12-25, Man Hung-Coeng:
 *  01. Create.
 *
 * >>> 2021-12-27, Man Hung-Coeng:
 *  01. Eliminate errors appeared in terminal
 *      while sig_global_init() is used on OS X.
 *
 * >>> 2022-01-03, Man Hung-Coeng:
 *  01. Add a function pointer to the parameter list of sig_register()
 *      to support user-defined operations.
 *  02. Correct wrong outputs from sig_name_to_number()
 *      when converting SIGRTMAX-1 and SIGRTMAX.
 *
 * >>> 2022-01-08, Man Hung-Coeng:
 *  01. Fix the out-of-bounds error in sig_error().
 *  02. Improve code style according to suggestions from cppcheck.
 *
 * >>> 2023-11-08, Man Hung-Coeng:
 *  01. Do type casting to result of calloc() to eliminate the warning
 *      reported by YouCompleteMe plugin.
 *  02. Comment out the pragma message of using new sigaction().
 *
 * >>> 2023-12-24, Man Hung-Coeng:
 *  01. Add several functions for simple application usage.
 */

