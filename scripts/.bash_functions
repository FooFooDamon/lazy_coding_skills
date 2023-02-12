#!/bin/bash

#
# Useful functions.
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
    errcode()
    {
        if [ $1 -eq 0 ]; then
            echo "OK"
        elif [ $1 -eq 1 ]; then
            echo "Catchall for general errors"
        elif [ $1 -eq 2 ]; then
            echo "Misuse of shell built-in commands"
        elif [ $1 -eq 126 ]; then
            echo "Not an executable, or permission denied"
        elif [ $1 -eq 127 ]; then
            echo "Command not found"
        elif [ $1 -eq 128 ]; then
            echo "Invalid argument"
        elif [ $1 -ge 64 ] && [ $1 -le 78 ]; then
            local _errmsg=$(grep "\<$1\>" /usr/include/sysexits.h | grep -v "\<EX__BASE\>\|\<EX__MAX\>" | sed "s/.*\/\*\(.*\)\*\//\1/")
            echo "${_errmsg}"
        elif [ $1 -gt 128 ] && [ $1 -le 192 ]; then
            local _signum=$(($1 - 128))
            local _signame=$(kill -l | grep "\<${_signum}\>)" | sed "/.*\<${_signum}\>)[ \t]\{0,\}\([A-Za-z0-9+-]*\).*/s//\1/g")
            echo "Interrupted or killed by signal ${_signame}"
        else
            echo "Unknown error"
        fi
    }

    lchelp()
    {
        printf '\nALIASES:\n'
        grep "alias " "${LAZY_CODING_HOME}/scripts/.bash_aliases" \
            | awk -F = '{ print $1 }' | awk '{ printf("    %s\n", $2) }' | sort
        echo '-- Run "alias <name>" to see definition of an alias, e.g. alias diff'

        printf '\nFUNCTIONS:\n'
        grep '^[ \t]*[a-zA-Z0-9_-]\+()$' "${LAZY_CODING_HOME}/scripts/.bash_functions" | sed 's/()//' | sort
        echo '-- Run "declare -f <name>" to see definition of a function, e.g. declare -f lchelp'

        printf '\nVARIABLES:\n'
        grep '^[ \t]*export [a-zA-Z0-9_-]\+=' "${LAZY_CODING_HOME}/scripts/.bash_variables" \
            | awk -F = '{ print $1 }' | sed 's/^[ \t]*export \(.*\)/    \1/' | sort | uniq
        echo '-- Run "echo ${<name>}" to see value of a variable, e.g. echo ${SHELL}'

        printf '\n'
    }

    man2pdf()
    {
        man -t "$1" | ps2pdf - "${1}.pdf"
    }
fi

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2023-02-12, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

