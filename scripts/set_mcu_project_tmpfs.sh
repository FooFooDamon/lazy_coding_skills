#!/bin/bash

# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) 2026 Man Hung-Coeng <udc577@126.com>
# All rights reserved.
#

usage()
{
    printf "\n$(basename $0) - Set TMPFS-based build directories for an MCU project\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...] /path/to/MCU/project [TMPFS-mount-point]\n"
    printf "  -h, --help     Show this help message.\n"
    printf "  -v, --version  Show version info.\n"
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) ~/src/STM32CubeIDE/led_blink\n"
    printf "  2. $(basename $0) ~/src/STM32CubeIDE/led_blink/src\n"
    printf "  3. $(basename $0) ~/src/STM32CubeIDE/led_blink /tmp\n"
    printf "  3. $(basename $0) ~/src/STM32CubeIDE/led_blink/src /dev/shm\n"
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
    printW "$(${DATETIME_CMD}): $(basename $0): Script was interrupted."
    exit 1
}

handle_sigQUIT()
{
    printW "$(${DATETIME_CMD}): $(basename $0): Script quit."
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

if [ $# -lt 1 ]; then
    printE "*** MCU project path not specified!"
    echo "Run with -h option to see detailed usage." >&2
    exit 1
fi

src_root="$(realpath $1)"
proj_name=$(basename ${src_root})

if [ $(echo "${proj_name}" | grep -icE "^(src|source)[s]?$") -gt 0 ]; then
    proj_name=$(basename $(dirname ${src_root}))
else
    if [ -d "${src_root}/src/Debug" -o -d "${src_root}/src/Release" ]; then
        src_root="${src_root}/src"
    fi
fi

if [ -z "${proj_name}" -o "${proj_name}" = "/" ]; then
    printE "*** Cannot determine project name!"
    exit 1
fi

if [ ! -d "${src_root}/Debug" -a ! -d "${src_root}/Release" ]; then
    printE "*** Cannot find Debug or Release directory in ${src_root}"
    exit 1
fi

owner=$(stat --format=%U "${src_root}")

[ -n "$2" ] && tmpfs_mountpoint="$2" || tmpfs_mountpoint=/tmp

for build_type in Debug Release
do
    tmpfs_path="${tmpfs_mountpoint}/lcs-mcu/${proj_name}/${build_type}"
    proj_path="${src_root}/${build_type}"

    df -a 2> /dev/null | grep --color=auto "${proj_path}$" >&2 && continue

    for p in "${tmpfs_path}" "${proj_path}"
    do
        if [ ! -d "${p}" ]; then
            mkdir -p "${p}"
            chown ${owner}:${owner} -R "${p}"
        fi
    done

    sudo mount --bind "${tmpfs_path}" "${proj_path}" && echo "${tmpfs_path} -> ${proj_path}"
done

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2026-08-17, Man Hung-Coeng <udc577@126.com>:
#   01. Initial commit.
#

