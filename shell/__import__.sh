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

if [ -n "${LAZY_CODING_HOME}" ]; then
    set -eu

    export PATH=${PATH}:${LAZY_CODING_HOME}/shell:${LAZY_CODING_HOME}/shell/private

    for i in .bash_aliases .bash_variables .bash_functions .bash_completions .bash_openup
    do
        [ -f "${LAZY_CODING_HOME}/shell/${i}" ] && . "${LAZY_CODING_HOME}/shell/${i}"
        [ -f "${LAZY_CODING_HOME}/shell/private/${i}" ] && . "${LAZY_CODING_HOME}/shell/private/${i}"
    done

    echo "-- Welcome to use Lazy-Coding-Skills (懒编程秘笈). --"
    echo " * Author: Man Hung-Coeng"
    echo " * E-Mail: udc577@126.com"
    echo " * GitHub: FooFooDamon"
    echo "-- Less code, better world!  Please try and enjoy! --"

    set +eu
fi

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-02-11, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

