#!/bin/bash

#
# Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

usage()
{
    printf "\n$(basename $0) - Get IOMUX pins of a specific functionality for ARM series processor\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...] <cpu> <muxname> [path to linux kernel source root directory]\n"
    printf "  -h, --help        Show this help message.\n"
    printf "  -v, --version     Show version info.\n"
    printf '  -l, --list        List MUX keywords if specifying "cpu", or list supported CPUs otherwise.\n'
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) -l\n"
    printf "  2. $(basename $0) -l imx6ull\n"
    printf "  3. $(basename $0) imx6ull ENET1\n"
    printf "  4. $(basename $0) imx6ull ENET1 ~/src/linux\n"
    printf "\n"
}

version()
{
    grep "^# >>> V[0-9.]\+[ ]*|" "$0" | tail -n 1 | sed 's/.*\(V[0-9.]\+[ ]*|[0-9-]\+\),.*/\1/'
}

printW()
{
    printf "\e[0;33m$*\e[0m\n" >&2
}

printE()
{
    printf "\e[0;31m$*\e[0m\n" >&2
}

eexit()
{
    [ $# -gt 0 ] && printE "$*"
    exit 1
}

DATETIME_CMD="date +%Y-%m-%d_%H:%M:%S.%N"

handle_sigINT()
{
    printW "$(${DATETIME_CMD}): $(basename $0): Script will exit soon."
    exit 1
}

handle_sigQUIT()
{
    printW "$(${DATETIME_CMD}): $(basename $0): Script will exit soon."
    exit 1
}

SIGNAL_ITEMS=(INT QUIT)
for i in ${SIGNAL_ITEMS[@]}
do
    trap "handle_sig${i}" ${i}
done

[ "$0" = "$(basename $0)" ] && THIS_DIR=$(dirname $(which $0)) || THIS_DIR=$(dirname $0)
list_flag=0
arguments=()

for i in "$@"
do
    if [ "${i}" = "-h" ] || [ "${i}" = "--help" ]; then
        usage
        exit 0
    elif [ "${i}" = "-v" ] || [ "${i}" = "--version" ]; then
        version
        exit 0
    elif [ "${i}" = "-l" ] || [ "${i}" = "--list" ]; then
        list_flag=1
    else
        arguments[${#arguments[@]}]="${i}"
    fi
done

set -e

if [ $list_flag -eq 1 ] && [ ${#arguments[@]} -eq 0 ]; then
    ls ${THIS_DIR}/.script_as_config/arch/arm/ | xargs -I {} basename {} .cfg.sh
    exit 0
fi

[ ${#arguments[@]} -gt 0 ] && CPU=${arguments[0]} || eexit '*** Missing "cpu" argument! Run with "-h" to see the usage.'
[ ${#arguments[@]} -gt 1 ] && MUXNAME=${arguments[1]} \
    || ([ $list_flag -eq 1 ] || eexit '*** Missing "muxname" argument! Run with "-h" to see the usage.')
[ ${#arguments[@]} -gt 2 ] && KERNEL_ROOT=${arguments[2]} || KERNEL_ROOT=${HOME}/src/linux

. ${THIS_DIR}/.script_as_config/arch/arm/${CPU}.cfg.sh || eexit "*** Incorrect or unsupported CPU: ${CPU}"

MASTER_GROUPS=(
    $(grep "^#define[[:space:]]\+${MUX_PREFIX}" ${HEADER_FILES[@]} \
        | awk '{ print $2 }' | sed 's/^[^_]\+_PAD_\([^_]\+\)_.\+/\1/' | sort | uniq)
)
RESULT_GROUPS=(
    $(grep "^#define[[:space:]]\+${MUX_PREFIX}" ${HEADER_FILES[@]} \
        | awk '{ print $2 }' | sed 's/^.\+__\([^_]\+\)_.\+/\1/' | sort | uniq)
)

imx6ul_mux_groupby()
{
    echo ${MASTER_GROUPS[@]} ${RESULT_GROUPS[@]} ${CALIBRATED_GROUPS[@]} \
        | sed 's/ /\n/g' | sort | uniq | grep -v "${IGNORED_GROUPS_REGEX}"
}

imx6ull_mux_groupby()
{
    imx6ul_mux_groupby
}

imx6ul_mux_query()
{
    [ $(echo ${!GROUP_ALIASES[@]} | grep -c "\<${MUXNAME}\>") -eq 0 ] && MUX_ALIAS=${MUXNAME} \
        || MUX_ALIAS=${GROUP_ALIASES[${MUXNAME}]}

    if [ "${MUXNAME}" = "BOOT" ]; then
        REGEX="^#define[[:space:]]\+${MUX_PREFIX}${MUXNAME}_"
    elif [ "${MUXNAME:0:4}" = "GPIO" ]; then
        REGEX="^#define[[:space:]]\+${MUX_PREFIX}.\+__${MUX_ALIAS}"
    else
        [ $(echo ${MASTER_GROUPS[@]} | sed 's/ /\n/g' | grep -c "\<${MUXNAME}\>") -eq 0 ] \
            && IN_MASTER_PART=0 || IN_MASTER_PART=1
        [ $(echo ${RESULT_GROUPS[@]} | sed 's/ /\n/g' | grep -c "\<${MUX_ALIAS}\>") -eq 0 ] \
            && IN_RESULT_PART=0 || IN_RESULT_PART=1

        if [ "${IN_MASTER_PART}${IN_RESULT_PART}" = "00" ]; then
            eexit "*** Invalid muxname: ${MUXNAME}"
        #elif [ "${IN_MASTER_PART}${IN_RESULT_PART}" = "11" ]; then
        #    REGEX="^#define[[:space:]]\+${MUX_PREFIX}${MUXNAME}_.\+__${MUX_ALIAS}"
        #elif [ ${IN_MASTER_PART} -eq 1 ]; then
        #    REGEX="^#define[[:space:]]\+${MUX_PREFIX}${MUXNAME}_"
        else
            REGEX="^#define[[:space:]]\+${MUX_PREFIX}.\+__${MUX_ALIAS}"
        fi
    fi

    grep -nH "${REGEX}" ${HEADER_FILES[@]} | awk '{ print $1, $2 }' \
        | sort -k 2 | uniq | sed 's/#define//' | column -t \
        | grep --color=auto "__${MUX_ALIAS}"
}

imx6ull_mux_query()
{
    imx6ul_mux_query
}

[ $list_flag -eq 1 ] && ${CPU}_mux_groupby || ${CPU}_mux_query

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2023-04-22, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

