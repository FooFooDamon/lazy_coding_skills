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
    printf "\n$(basename $0) - Trap GUI apps of BAT (and disgusting companies like them).\n"
    printf "\nUSAGE: [BAT_IMAGE=<LINUX IMAGE>] $(basename $0) [OPTIONS ...] [CONTAINER NAME]\n"
    printf "  -h, --help     Show this help message.\n"
    printf "  -v, --version  Show version info.\n"
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0)\n"
    printf "  2. $(basename $0) fuck_bat_again\n"
    printf "  3. BAT_IMAGE=ubuntu:20.04 $(basename $0) fuck_bat_twice\n"
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

BAT_CONTAINER=fuck_bat

for i in "$@"
do
    if [ "${i}" = "-h" ] || [ "${i}" = "--help" ]; then
        usage
        exit 0
    elif [ "${i}" = "-v" ] || [ "${i}" = "--version" ]; then
        version
        exit 0
    else
        BAT_CONTAINER=${i}
    fi
done

if [ $(docker ps -a --filter name="^/${BAT_CONTAINER}$" --format '{{.Names}}' | wc -l) -gt 0 ]; then
    docker start -i ${BAT_CONTAINER}
else
    if [ -z "${BAT_IMAGE}" ]; then
        docker images --format '{{.Repository}}:{{.Tag}}' | grep '^ubuntu:[0-9]\+\.[0-9]\+' | sort -nr | while read i
        do
            BAT_IMAGE=${i}
            break
        done
    fi
    [ -n "${BAT_IMAGE}" ] || BAT_IMAGE=ubuntu:22.04

    [ -n "${SHARED_DIR}" ] || SHARED_DIR=${HOME}/${BAT_CONTAINER}
    [ -d ${SHARED_DIR} ] || eexit "*** Directory not found: ${SHARED_DIR}\n*** Create it, or make such a symbolic link."

    [[ -z "$USER" || "$USER" = "root" ]] && SUDO="" || SUDO=sudo

    ${SUDO} openvt -- docker run --name=${BAT_CONTAINER} -ti \
        --ulimit core=-1 \
        --network=host \
        -v /etc/localtime:/etc/localtime:ro \
        -v /etc/timezone:/etc/timezone:ro \
        -v $(realpath ${SHARED_DIR}):$(realpath ${SHARED_DIR}) \
        -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
        -v /run/dbus/system_bus_socket:/run/dbus/system_bus_socket:ro \
        -v /run/user/${UID}:/run/user/${UID}:ro \
        -e DISPLAY=unix${DISPLAY} \
        -e DBUS_SESSION_BUS_ADDRESS=${DBUS_SESSION_BUS_ADDRESS} \
        ${BAT_IMAGE} \
        bash

    [ $? -eq 0 ] || exit $?

    echo "Cage for BAT has been created successfully, enter it with: docker attach ${BAT_CONTAINER}"
    echo "For more settings, see: https://foofoodamon.github.io/docker_trapbat.html"
fi

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2023-11-24, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

