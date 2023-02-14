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

DATETIME_CMD="date +%Y-%m-%d_%H:%M:%S.%N"

usage()
{
    printf "\n$(basename $0) - Enhanced verson of safe-rm.\n"
    printf "\nUSAGE: Almost same as safe-rm, except for cases below.\n"
    printf "\nNOTES:\n"
    printf "  1. Wildcard matching is not supported yet (e.g. /usr/lib/*).\n"
    printf "  2. A path to be deleted can not start with \"-\".\n"
    printf "  3. A path might not be protected while the deletion is done to its parent directory.\n"
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

for i in "$@"
do
    if [ "${i}" = "-h" ] || [ "${i}" = "--help" ]; then
        usage
        exit 0
    elif [ "${i}" = "-v" ] || [ "${i}" = "--version" ]; then
        version
        exit 0
    else
        continue
    fi
done

GLOBAL_CONFIG=/etc/safe-rm.conf # Use the configuration of safe-rm.
USER_CONFIG="${HOME}/etc/safer-rm"
DEFAULT_PROTECTED_DIRS=(
    "/bin"
    "/boot"
    "/dev"
    "/etc"
    "/lib"
    "/lib32"
    "/libx32"
    "/lib64"
    "/proc"
    "/run"
    "/sbin"
    "/sys"
    "/usr"
    "/var"
)

[ -e "${HOME}/etc" ] || mkdir "${HOME}/etc" || exit 1

if [ ! -f ${GLOBAL_CONFIG} ] && [ ! -f "${USER_CONFIG}" ]; then
    touch "${USER_CONFIG}"
    for i in "${DEFAULT_PROTECTED_DIRS[@]}"
    do
        echo "${i}" >> "${USER_CONFIG}"
    done
fi

PROTECTED_LIST_FILE="/tmp/$(basename $0 .sh).protectedlist.$(date +%Y%m%d%H%M%S%N)"
cat ${GLOBAL_CONFIG} "${USER_CONFIG}" > ${PROTECTED_LIST_FILE} 2> /dev/null

cmd_params=("$@")
param_index=0

for param in "${cmd_params[@]}"
do
    [ "${param:0:1}" != "-" ] && param_real_path="$(realpath -e "$param")" || param_real_path=""
    if [ -n "${param_real_path}" ]; then
        while read protected_item
        do
            item_real_path="$(realpath -e "${protected_item}" 2> /dev/null)"
            [ -n "${item_real_path}" ] || continue
            [ "${item_real_path}" != "/" ] || continue

            item_len=${#item_real_path}
            if [ "${param_real_path:0:$item_len}" = "${item_real_path}" ]; then
                printW "$(basename $0): skipping: ${cmd_params[$param_index]}"
                unset cmd_params[$param_index]
                break
            fi
        done < ${PROTECTED_LIST_FILE}
    fi
    param_index=$(($param_index + 1))
done

/bin/rm ${PROTECTED_LIST_FILE}

if [ ${#cmd_params[@]} -gt 0 ]; then
    echo "$(basename $0): /bin/rm ${cmd_params[@]}" >&2
    /bin/rm -r "${cmd_params[@]}"
fi

#
# ================
#   CHANGE LOG
# ================
#
# >>> V0.9.0|2023-02-12, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> V0.9.1|2023-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Add 3 new functions: printW(), printE() and eexit().
#

