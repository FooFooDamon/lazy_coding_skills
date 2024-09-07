/*
 * TODO: Brief description of this file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
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

//#include "${SELF_HEADER}.hpp"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>

// Must be coincident with the copyright info at the beginning of this file.
#ifndef COPYRIGHT_STRING
#define COPYRIGHT_STRING                "Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>\n" \
                                        "Licensed under the Apache License, Version 2.0"
#endif

#ifndef BRIEF_INTRO
#define BRIEF_INTRO                     "FIXME: Customize brief introduction of this program"
#endif

#ifndef USAGE_FORMAT
#define USAGE_FORMAT                    "[OPTION...] [FILE...]"
#endif

#define __CSTR(x)                       #x
#define CSTR(x)                         __CSTR(x)

#ifndef MAJOR_VER
#define MAJOR_VER                       0
#endif

#ifndef MINOR_VER
#define MINOR_VER                       1
#endif

#ifndef PATCH_VER
#define PATCH_VER                       0
#endif

#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION                 CSTR(MAJOR_VER) "." CSTR(MINOR_VER) "." CSTR(PATCH_VER)
#endif

#ifndef __VER__
#define __VER__                         "<none>"
#endif

#define BIZ_TYPE_CANDIDATES             "normal,test"
#define BIZ_TYPE_DEFAULT                "normal"

#ifdef HAS_CONFIG_FILE
#ifndef DEFAULT_CONF_FILE
#define DEFAULT_CONF_FILE               "config.ini"
#endif
#endif

#ifdef HAS_LOGGER

#ifndef DEFAULT_LOG_FILE
#define DEFAULT_LOG_FILE                "unnamed.log"
#endif

#ifndef LOG_LEVEL_CANDIDATES
#define LOG_LEVEL_CANDIDATES            "debug,info,notice,warning,error,critical"
#endif

#ifndef LOG_LEVEL_DEFAULT
#define LOG_LEVEL_DEFAULT               "warning"
#endif

#endif // #ifdef HAS_LOGGER

typedef struct cmd_args
{
    std::vector<std::string> orphan_args;
    std::string biz;
    std::string config_file;
#ifdef HAS_LOGGER
    std::string log_file;
    std::string log_level;
#else
    bool verbose;
    bool debug;
#endif
    // FIXME: Add more fields according to your need, and delete this comment line.
} cmd_args_t;

cmd_args_t parse_cmdline(int argc, char **argv)
{
    const struct
    {
        struct option content;
        const char* const description;
    } OPTION_RULES[] = {
        // name (long option), has_arg (no_*, required_* or optional_*), flag (fixed to NULL), val (short option or 0)
        // description string with proper \t and \n characters
        {
            { "help", no_argument, nullptr, 'h' },
            "\t\tShow this help message."
        },
        {
            { "copyright", no_argument, nullptr, 0 },
            "\tShow copyright info."
        },
        {
            { "version", no_argument, nullptr, 'v' },
            "\t\tShow product version number."
        },
        {
            { "vcs-version", no_argument, nullptr, 0 },
            "\tShow version number generated by version control system."
        },
#ifdef HAS_LOGGER
        {
            { "logfile", required_argument, nullptr, 0 },
            " /PATH/TO/LOG/FILE\n\t\t\tSpecify log file. Default to " DEFAULT_LOG_FILE "."
        },
        {
            { "loglevel", required_argument, nullptr, 0 },
            " {" LOG_LEVEL_CANDIDATES "}\n\t\t\tSpecify log level. Default to " LOG_LEVEL_DEFAULT "."
        },
#else
        {
            { "verbose", no_argument, nullptr, 'V' },
            "\t\tRun in verbose mode to produce more messages."
        },
        {
            { "debug", no_argument, nullptr, 0 },
            "\t\tProduce all messages of verbose mode, plus debug ones."
        },
#endif
#ifdef HAS_CONFIG_FILE
        {
            { "config", required_argument, nullptr, 'c' },
            " /PATH/TO/CONFIG/FILE\n\t\t\tSpecify configuration file. Default to " DEFAULT_CONF_FILE "."
        },
#endif
        {
            { "biz", required_argument, nullptr, 'b' },
            " {" BIZ_TYPE_CANDIDATES "}\n\t\t\tSpecify biz type. Default to " BIZ_TYPE_DEFAULT "."
        },
        // FIXME: Add more items according to your need, and delete this comment line.
    };
    struct option long_options[sizeof(OPTION_RULES) / sizeof(OPTION_RULES[0]) + 1];
    std::map<std::string, char> abbr_map;
    std::string short_options;
    cmd_args_t result = {};

    /*
     * NOTE: This block is part of fixed algorithm, DO NOT modify or delete it unless you have a better solution.
     */
    for (size_t i = 0; i < sizeof(OPTION_RULES) / sizeof(OPTION_RULES[0]); ++i)
    {
        const struct option &opt = OPTION_RULES[i].content;

        long_options[i] = opt;

        abbr_map[opt.name] = opt.val;

        if (opt.val <= 0)
            continue;

        short_options += opt.val;
        if (no_argument == opt.has_arg)
            continue;
        else if (required_argument == opt.has_arg)
            short_options += ':';
        else
            short_options += "::";
    }
    long_options[sizeof(long_options) / sizeof(long_options[0]) - 1] = {}; // sentinel

    /*
     * Set some default option values here.
     */
    result.biz = BIZ_TYPE_DEFAULT;
#ifdef HAS_CONFIG_FILE
    result.config_file = DEFAULT_CONF_FILE;
#endif
#ifdef HAS_LOGGER
    result.log_file = DEFAULT_LOG_FILE;
    result.log_level = LOG_LEVEL_DEFAULT;
#endif
    // FIXME: Add more settings according to your need, and delete this comment line.

    while (true)
    {
        int option_index = 0;
        int c = getopt_long(argc, argv, short_options.c_str(), long_options, &option_index);

        if (-1 == c) // all parsed
            break;
        else if (0 == c)
            c = abbr_map[long_options[option_index].name]; // re-mapped to short option and continue
        else
        {
            // empty block for the integrity of if-else statement
        } // continue as follows

        if (0 == c) // for long-only options
        {
            const char *long_opt = long_options[option_index].name;

            if (0 == strcmp(long_opt, "copyright"))
            {
                printf("%s\n", COPYRIGHT_STRING);
                exit(EXIT_SUCCESS);
            }
            else if (0 == strcmp(long_opt, "vcs-version"))
            {
                printf("%s\n", __VER__);
                exit(EXIT_SUCCESS);
            }
#ifdef HAS_LOGGER
            else if (0 == strcmp(long_opt, "logfile"))
                result.log_file = optarg;
            else if (0 == strcmp(long_opt, "loglevel"))
                result.log_level = optarg;
#else
            else if (0 == strcmp(long_opt, "debug"))
                result.debug = true;
#endif
            // FIXME: Add more branches according to your need, and delete this comment line.
            else
            {
                fprintf(stderr, "*** Are you forgetting to handle --%s option??\n", long_opt);
                exit(EINVAL);
            }
        } // c == 0: for long-only options
        else if (abbr_map["biz"] == c)
            result.biz = optarg;
#ifdef HAS_CONFIG_FILE
        else if (abbr_map["config"] == c)
            result.config_file = optarg;
#endif
        // FIXME: Add more branches according to your need, and delete this comment line.
        else if (abbr_map["help"] == c) // NOTE: Branches below rarely need customizing.
        {
            const char *slash = strrchr(argv[0], '/');
            const char *program_name = (nullptr == slash) ? argv[0] : (slash + 1);

            printf("\n%s - %s\n\nUsage: %s %s\n\n", program_name, BRIEF_INTRO, program_name, USAGE_FORMAT);
            for (const auto &rule : OPTION_RULES)
            {
                const struct option &opt = rule.content;
                bool has_short = (opt.val > 0);

                printf("  %c%c%c --%s%s\n\n", (has_short ? '-' : ' '), (has_short ? (char)opt.val : ' '),
                    (has_short ? ',' : ' '), opt.name, rule.description);
            }
            exit(EXIT_SUCCESS);
        }
        else if (abbr_map["version"] == c)
        {
            printf("%s\n", PRODUCT_VERSION);
            exit(EXIT_SUCCESS);
        }
#ifndef HAS_LOGGER
        else if (abbr_map["verbose"] == c)
            result.verbose = true;
#endif
        else if ('?' == c || ':' == c)
            exit(EINVAL); // getopt_long() will print the reason.
        else // This case should never happen.
        {
            fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
            exit(EINVAL);
        }
    } // while (true)

    while (optind < argc)
    {
        result.orphan_args.push_back(argv[optind++]);
    }
    // FIXME: Decide how to use orphan_args, and delete this comment line.

    return result;
} // cmd_args_t parse_cmdline(int argc, char **argv)

#undef CSTR

template<typename T>
static void assert_comparable_arg(const char *name, const T &val, const T &min, const T &max)
{
    assert(max > min);
    if (val < min || val > max)
    {
        std::cerr << "*** The specified " << name << " exceeds range[" << min << ", " << max << "]!" << std::endl;
        exit(EINVAL);
    }
}

void assert_parsed_args(const cmd_args_t &args)
{
    const struct
    {
        const char *name;
        const char *val;
    } required_str_args[] = {
        { "biz type", args.biz.c_str() },
#ifdef HAS_CONFIG_FILE
        { "config file", args.config_file.c_str() },
#endif
#ifdef HAS_LOGGER
        { "log file", args.log_file.c_str() },
        { "log level", args.log_level.c_str() },
#endif
        // FIXME: Add more items according to your need, and delete this comment line.
    };
    const struct
    {
        const char *name;
        const char *val;
        const char *candidates;
    } enum_str_args[] = {
        { "biz type", args.biz.c_str(), BIZ_TYPE_CANDIDATES },
#ifdef HAS_LOGGER
        { "log level", args.log_level.c_str(), LOG_LEVEL_CANDIDATES },
#endif
        // FIXME: Add more items according to your need, and delete this comment line.
    };

    for (const auto &arg : required_str_args)
    {
        if (nullptr == arg.val || '\0' == arg.val[0])
        {
            fprintf(stderr, "*** %s is null or not specified!\n", arg.name);
            exit(EINVAL);
        }
    }

    for (const auto &arg : enum_str_args)
    {
        const char *ptr = (arg.val && arg.val[0]) ? strstr(arg.candidates, arg.val) : nullptr;
        size_t len = ptr ? strlen(arg.val) : 0;

        if (nullptr == ptr || (',' != ptr[len] && '\0' != ptr[len]))
        {
            fprintf(stderr, "*** Invalid %s: %s\nMust be one of {%s}\n", arg.name, arg.val, arg.candidates);
            exit(EINVAL);
        }
    }

    // FIXME: Add more validations according to your need, and delete this comment line.
} // void assert_parsed_args(const cmd_args_t &args)

#define todo()                          fprintf(stderr, __FILE__ ":%d %s(): todo ...\n", __LINE__, __func__)

typedef struct conf_file
{
#ifdef HAS_CONFIG_FILE
    std::string path;
    // Add more fields according to your need, and delete this comment line.
#endif
} conf_file_t;

int load_config_file(const char *path, conf_file_t &result)
{
#ifdef HAS_CONFIG_FILE
    todo();
#endif
    return 0;
}

void unload_config_file(conf_file_t &result)
{
#ifdef HAS_CONFIG_FILE
    todo();
#endif
}

int logger_init(const cmd_args_t &args, const conf_file_t &conf)
{
#ifdef HAS_LOGGER
    todo();
#endif
    return 0;
}

void logger_finalize(void)
{
#ifdef HAS_LOGGER
    todo();
#endif
}

int register_signals(const cmd_args_t &args, const conf_file_t &conf)
{
#ifdef NEED_OS_SIGNALS
    todo();
#endif
    return 0;
}

#define BIZ_FUN_ARG_LIST                int argc, char **argv, const cmd_args_t &parsed_args, const conf_file_t &conf
#define DECLARE_BIZ_FUN(name)           int name(BIZ_FUN_ARG_LIST)
#define BIZ_FUN(name)                   name
typedef int (*biz_func_t)(BIZ_FUN_ARG_LIST);

static DECLARE_BIZ_FUN(normal_biz)
{
    todo();

    return EXIT_SUCCESS;
}

static DECLARE_BIZ_FUN(test_biz)
{
    todo();

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    cmd_args_t parsed_args = parse_cmdline(argc, argv);
    conf_file_t conf;
    std::map<std::string, biz_func_t> biz_handlers = {
        { "normal", BIZ_FUN(normal_biz) },
        { "test", BIZ_FUN(test_biz) },
    };
    biz_func_t biz_func = nullptr;
    int ret;

    assert_parsed_args(parsed_args);

    if (nullptr == (biz_func = biz_handlers[parsed_args.biz]))
    {
        fprintf(stderr, "*** Biz[%s] is not supported yet!\n", parsed_args.biz.c_str());
        return ENOTSUP;
    }

    if ((ret = load_config_file(parsed_args.config_file.c_str(), conf)) < 0)
        return -ret;

    if ((ret = logger_init(parsed_args, conf)) < 0)
        goto lbl_unload_conf;

    if ((ret = register_signals(parsed_args, conf)) < 0)
        goto lbl_finalize_log;

    ret = biz_func(argc, argv, parsed_args, conf);

lbl_finalize_log:
    logger_finalize();

lbl_unload_conf:
    unload_config_file(conf);

    return abs(ret);
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
