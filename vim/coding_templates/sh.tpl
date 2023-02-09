#!/bin/bash

#
# TODO: Brief description of this script.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
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
    echo "$(basename $0) - <Brief description of this script>"
    echo "Usage: $(basename $0) <Usage format of this script>"
    echo "Example 1: $(basename $0) <An example showing how actual arguments should be like>"
    # TODO: Put more examples if necessary.
}

version()
{
    grep "^# >>> V[0-9.]\+[ ]*|" "$0" | tail -n 1 | sed 's/.*\(V[0-9.]\+[ ]*|[0-9-]\+\),.*/\1/'
}

handle_sigINT()
{
    # TODO: Define what to do here when SIGINT arises.
    printf "\e[0;33m$(${DATETIME_CMD}): $(basename $0): Script will exit soon.\e[0m\n" >&2
    exit 1
}

handle_sigQUIT()
{
    # TODO: Define what to do here when SIGQUIT arises.
    printf "\e[0;33m$(${DATETIME_CMD}): $(basename $0): Script will exit soon.\e[0m\n" >&2
    exit 1
}

#
# Register signals.
#
SIGNAL_ITEMS=(INT QUIT)
for i in ${SIGNAL_ITEMS[@]}
do
    trap "handle_sig${i}" ${i}
done

#
# Parse command line options.
#
for i in "$@"
do
    if [ "${i}" = "-h" ] || [ "${i}" = "--help" ]; then
        usage
        exit 0
    elif [ "${i}" = "-v" ] || [ "${i}" = "--version" ]; then
        version
        exit 0
    else
        continue # Or whatever you want.
    fi
done

# TODO: Your own stuff

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
#   01. Create.
#
