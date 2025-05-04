#!/bin/bash

# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#

usage()
{
    printf "\n$(basename $0) - <Brief description of this script>\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...]\n"
    printf "  -h, --help     Show this help message.\n"
    printf "  -v, --version  Show version info.\n"
    # TODO: Put more options if any.
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) <An example showing how actual arguments should be like>\n"
    # TODO: Put more examples if necessary.
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
    # TODO: Define what to do here when SIGINT arises.
    printW "$(${DATETIME_CMD}): $(basename $0): Script will exit soon."
    exit 1
}

handle_sigQUIT()
{
    # TODO: Define what to do here when SIGQUIT arises.
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
        continue # TODO: Or whatever you want.
    fi
done

# TODO: Your own stuff

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
#   01. Initial commit.
#
