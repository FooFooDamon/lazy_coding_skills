#!/bin/bash

# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
# All rights reserved.
#

usage()
{
    printf "\n$(basename $0) - Show current utilization, frequency and temperature of each CPU core\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...]\n"
    printf "  -h, --help     Show this help message.\n"
    printf "  -v, --version  Show version info.\n"
    #printf "  -i, --interval SECONDS\n"
    #printf "                 Specify update interval.\n"
    #printf "  -n NUMBER      Specify the maximum number of iterations.\n"
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0)\n"
    #printf "  2. $(basename $0) -n 5\n"
    #printf "  3. $(basename $0) -n -1 -i 1.5\n"
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

sysfs_root_dir=/sys/devices/system/cpu
# This works on X86 only.
#freq_array=($(grep '^cpu MHz' /proc/cpuinfo | awk '{ printf("%dMHz\n", $NF); }'))
freq_idx=0

top -b -n 1 -w 512 -1 | grep '%Cpu' | sed -e 's/,/, /g' -e 's/[ \t]\+\(%Cpu.\+\)/\n\1/g' \
    | awk '{ printf("%s: %4.1f%%\n", $1, 100 - $9); }' | sed 's/^%//' \
    | while read i
do
    #echo "${i} @ ${freq_array[${freq_idx}]}"
    max_freq=$(awk '{ printf("%.1fGHz\n", $1 / 1000000); }' ${sysfs_root_dir}/cpu${freq_idx}/cpufreq/cpuinfo_max_freq)
    cur_freq=$(awk '{ printf("%.1fGHz\n", $1 / 1000000); }' ${sysfs_root_dir}/cpu${freq_idx}/cpufreq/scaling_cur_freq)
    echo "${i} @ ${cur_freq} (max=${max_freq})"
    freq_idx=$((freq_idx + 1))
done | column -t -R 2

echo "--"
sensors -A 2> /dev/null | grep -i 'core' -A 1

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2025-04-05, Man Hung-Coeng <udc577@126.com>:
#   01. Initial commit.
#

