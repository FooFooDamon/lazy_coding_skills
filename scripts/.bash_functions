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

    get_url_by_linux_tag()
    {
        [ -n "${FUNCNAME}" ] || FUNCNAME="get_url_by_linux_tag"
        if [ $# -lt 1 ]; then
            echo "*** Usage: ${FUNCNAME} <tag name>" >&2
            echo "*** Example 1: ${FUNCNAME} v6.2.16" >&2
            echo "*** Example 2: ${FUNCNAME} v6.2-rc1" >&2
            return 1
        else
            echo "https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tag/?h=$1"
        fi
    }

    lchelp()
    {
        printf '\nALIASES:\n'
        grep "alias " "${LAZY_CODING_HOME}/scripts/.bash_aliases" \
            | awk -F = '{ print $1 }' | awk '{ printf(" %s\n", $2) }' | sort | nl -s . -w 3
        echo '-- Run "alias <name>" to see definition of an alias, e.g. alias diff'

        printf '\nFUNCTIONS:\n'
        grep '^[ \t]*[a-zA-Z0-9_-]\+()$' "${LAZY_CODING_HOME}/scripts/.bash_functions" \
            | sed 's/^[ \t]*\([^ \t()]\+\)()/ \1/' | sort | nl -s . -w 3
        echo '-- Run "declare -f <name>" to see definition of a function, e.g. declare -f lchelp'

        printf '\nVARIABLES:\n'
        grep '^[ \t]*export [a-zA-Z0-9_-]\+=' "${LAZY_CODING_HOME}/scripts/.bash_variables" \
            | awk -F = '{ print $1 }' | sed 's/^[ \t]*export \(.*\)/ \1/' | sort | uniq | nl -s . -w 3
        echo '-- Run "echo ${<name>}" to see value of a variable, e.g. echo ${SHELL}'

        printf '\nSCRIPTS:\n'
        cd "${LAZY_CODING_HOME}/scripts"
        ls *.exp *.sh *.py 2> /dev/null | grep -v "^_" | sed 's/\(.*\)/ \1/' | nl -s . -w 3
        cd - > /dev/null
        echo '-- Run "<name> -h" to see usage of a script, e.g. safer-rm.sh -h'

        printf '\n'
    }

    man2pdf()
    {
        if [ -z "$1" ]; then
            echo "*** Please specify a command!" >&2
            return 1
        else
            man -t "$1" | ps2pdf - "${1}.pdf"
        fi
    }

    play_abnormal_exit_audio()
    {
        [ $(echo $@ | grep -c "[ ]*-T[ ]*") -gt 0 ] && echo "Current date time: $(date '+%Y-%m-%d %H:%M:%S')" || :
        while true
        do
            play_abnormal_exit_audio_once || break
        done
    }

    play_abnormal_exit_audio_once()
    {
        [ $(echo $@ | grep -c "[ ]*-T[ ]*") -gt 0 ] && echo "Current date time: $(date '+%Y-%m-%d %H:%M:%S')" || :
        if [ -r $LAZY_CODING_HOME/res/custom/audio/abnormal_termination.flac ]; then
            mpv $LAZY_CODING_HOME/res/custom/audio/abnormal_termination.flac
        else
            mpv $LAZY_CODING_HOME/res/default/audio/abnormal_termination.flac
        fi
    }

    play_normal_exit_audio()
    {
        [ $(echo $@ | grep -c "[ ]*-T[ ]*") -gt 0 ] && echo "Current date time: $(date '+%Y-%m-%d %H:%M:%S')" || :
        while true
        do
            play_normal_exit_audio_once || break
        done
    }

    play_normal_exit_audio_once()
    {
        [ $(echo $@ | grep -c "[ ]*-T[ ]*") -gt 0 ] && echo "Current date time: $(date '+%Y-%m-%d %H:%M:%S')" || :
        if [ -r $LAZY_CODING_HOME/res/custom/audio/normal_termination.flac ]; then
            mpv $LAZY_CODING_HOME/res/custom/audio/normal_termination.flac
        else
            mpv $LAZY_CODING_HOME/res/default/audio/normal_termination.flac
        fi
    }

    remind_me_if_task_done()
    {
        [ -n "${FUNCNAME}" ] || FUNCNAME="remind_me_if_task_done"
        if [ $# -lt 1 ]; then
            echo "*** Usage: ${FUNCNAME} <grep-compatible regular expression>" >&2
            echo "*** Example: ${FUNCNAME} 'firefox'" >&2
            return 1
        else
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] Monitoring task[$1]."
            while true
            do
                [ $(ps -ef | grep -v " grep " | grep -c "$1") -gt 0 ] || break
                sleep 2
            done
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] Task[$1] done."
            play_normal_exit_audio
        fi
    }
fi

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-02-12, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2023-02-13, Man Hung-Coeng <udc577@126.com>:
#   01. Add a command line argument checking in man2pdf().
#
# >>> 2023-02-15, Man Hung-Coeng <udc577@126.com>:
#   01. Remove "set -e" to avoid weird sideeffects.
#
# >>> 2023-09-07, Man Hung-Coeng <udc577@126.com>:
#   01. Add play_*_exit_audio*() and remind_me_if_task_done().
#
# >>> 2023-10-10, Man Hung-Coeng <udc577@126.com>:
#   01. Add get_url_by_linux_tag().
#

