#!/bin/bash

#
# Copyright (c) 2024 Man Hung-Coeng <udc577@126.com>
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
    printf "\n$(basename $0) - Switch Linux kernel to specific version\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...] /path/to/kernel-image\n"
    printf "  -h, --help     Show this help message.\n"
    printf "  -v, --version  Show version info.\n"
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) /boot/vmlinuz-6.5.0-21-generic\n"
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

[ -n "$1" -a -e "$1" ] || eexit "*** Kernel image not specified or non-existent: $1"
IMG_PATH=$(echo "$1" | grep -v '\.old$')
[ -n "${IMG_PATH}" ] || eexit "*** You've specified a backup image. Choose one without .old suffix."

[ $(ls /etc/kernel/postinst.d/ | wc -l) -gt 0 ] || exit 1

[ "$(id -u)" -eq 0 -o "${USER}" = "root" ] && SUDO="" || SUDO="sudo"
KVER=$(basename "${IMG_PATH}" | sed 's/[^-]*-\(.*\)/\1/')

PATH="$PATH:/usr/sbin:/sbin" ${SUDO} run-parts --verbose --exit-on-error \
    --arg="${KVER}" --arg="${IMG_PATH}" /etc/kernel/postinst.d

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2024-03-26, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

