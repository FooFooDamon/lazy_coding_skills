#!/bin/bash

#
# Copyright (c) 2023-2024 Man Hung-Coeng <udc577@126.com>
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

time if [ -n "${LAZY_CODING_HOME}" ]; then
    [ -n "${LCS_IMPORT_COUNT}" ] || export LCS_IMPORT_COUNT=0

    grep "\(^\|:\)${LAZY_CODING_HOME}/scripts/private:${LAZY_CODING_HOME}/scripts\(:\|$\)" <<< "${PATH}" > /dev/null \
        || export PATH=${LAZY_CODING_HOME}/scripts/private:${LAZY_CODING_HOME}/scripts:${PATH}

    for i in .bash_aliases .bash_variables .bash_functions .bash_openup .bash_completions
    do
        [ -f "${LAZY_CODING_HOME}/scripts/${i}" ] && . "${LAZY_CODING_HOME}/scripts/${i}"
        [ -f "${LAZY_CODING_HOME}/scripts/private/${i}" ] && . "${LAZY_CODING_HOME}/scripts/private/${i}"
    done
    unset i

    if [ "$1" != "-q" -a "$1" != "--quiet" ]; then
        echo "-- Welcome to use Lazy-Coding-Skills (懒编程秘笈). --"
        echo " * Author: Man Hung-Coeng"
        echo " * E-Mail: udc577@126.com"
        echo " * GitHub: FooFooDamon"
        echo " * Tip-01: To get help, run: lchelp"
        echo " * Tip-02: To do terminal logging, run: tl"
        echo "-- Less code, better world!  Please try and enjoy! --"
    fi

    export LCS_IMPORT_COUNT=$((${LCS_IMPORT_COUNT} + 1))
fi >&2

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-02-11, Man Hung-Coeng <udc577@126.com>:
#   01. Initial release.
#
# >>> 2023-02-12, Man Hung-Coeng <udc577@126.com>:
#   01. Add two tips.
#
# >>> 2023-02-13, Man Hung-Coeng <udc577@126.com>:
#   01. Unset the loop counter "i" after use
#       to avoid variable pollution.
#   02. Display how much time on loading this script.
#
# >>> 2023-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Put .bash_completions after .bash_openup.
#
# >>> 2023-02-15, Man Hung-Coeng <udc577@126.com>:
#   01. Add a counter variable LCS_IMPORT_COUNT.
#   02. Remove "set -e -u" to avoid weird sideeffects.
#
# >>> 2024-06-01, Man Hung-Coeng <udc577@126.com>:
#   01. (Conditionally) Suppress, and redirect the welcome message.
#
# >>> 2024-08-15, Man Hung-Coeng <udc577@126.com>:
#   01. Prepend instead of appending script directories to PATH.
#

