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
    printf "\n$(basename $0) - Check if there're repeated IOMUX pins in a device tree file for ARM series processor\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...] <cpu> <path to .dts file> [path to linux kernel source root directory]\n"
    printf "  -h, --help        Show this help message.\n"
    printf "  -v, --version     Show version info.\n"
    printf '  -l, --list        List supported CPUs.\n'
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) -l\n"
    printf "  2. $(basename $0) imx6ull xx.dts\n"
    printf "  3. $(basename $0) imx6ull xx.dts ~/src/linux\n"
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
        ls ${THIS_DIR}/.script_as_config/arch/arm/ | xargs -I {} basename {} .cfg.sh
        exit 0
    else
        arguments[${#arguments[@]}]="${i}"
    fi
done

set -e

[ ${#arguments[@]} -gt 0 ] && CPU=${arguments[0]} || eexit '*** Missing "cpu" argument! Run with "-h" to see the usage.'
[ ${#arguments[@]} -gt 1 ] && DTS_FILE=${arguments[1]} || eexit '*** Missing "dts" argument! Run with "-h" to see the usage.'
[ ${#arguments[@]} -gt 2 ] && KERNEL_ROOT=${arguments[2]} || KERNEL_ROOT=${HOME}/src/linux

[ -r ${DTS_FILE} ] || eexit "*** File does not exist: ${DTS_FILE}"

. ${THIS_DIR}/.script_as_config/arch/arm/$(echo "${CPU}" | tr "A-Z" "a-z").cfg.sh \
    || eexit "*** Incorrect or unsupported CPU: ${CPU}"

grep "^#define[[:space:]]\+${MUX_PREFIX}" ${HEADER_FILES[@]} | awk '{ print $3 }' | sort | uniq | while read i
do
    printf "\e[0;33m>>> Checking IOMUX pin with address offset [${i}] ...\e[0m\n"
    grep "^#define[[:space:]]\+${MUX_PREFIX}" ${HEADER_FILES[@]} | awk "{ if (\"${i}\" == \$3) print \$2 }" | sort | uniq | while read j
    do
        #grep -n --color=auto "${j}" ${DTS_FILE} # TODO: Why THE FUCK does it ABORT in the beginning ?!!
        #cat -n ${DTS_FILE} | sed -n "/${j}/p" # Simple but not so pretty.
        #nl -ba -n'rn' -s: -w4 ${DTS_FILE} | sed -n "/${j}/p" # Simple but not so robust.
        sed -n "/${j}/{=;p}" ${DTS_FILE} | sed 'N; s/\n/: /' # A bit tricky.
    done
done

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2023-04-22, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

