#!/bin/bash

#
# Useful functions.
#
# Copyright (c) 2023-2026 Man Hung-Coeng <udc577@126.com>
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

    pdf_bookmark_get()
    {
        [ -n "${FUNCNAME}" ] || FUNCNAME="pdf_bookmark_get"

        if [ $# -lt 1 ]; then
            (
                echo "*** Insufficient arguments!"
                echo "Usage: ${FUNCNAME} <input-pdf> [<page-offset> [indent-width]]"
                echo "Example 1: ${FUNCNAME} \"\" # Show a simple example."
                echo "Example 2: ${FUNCNAME} my_pdf.pdf"
                echo "Example 3: ${FUNCNAME} my_pdf.pdf 13"
                echo "Example 4: ${FUNCNAME} my_pdf.pdf 0 2"
            ) >&2

            return 128
        fi

        local input_pdf="$1"

        if [ "${input_pdf}" != "" -a ! -f "${input_pdf}" ]; then
            echo "*** Input PDF file does not exist: ${input_pdf}" >&2
            return 1
        fi

        local page_offset=$(echo "$2" | grep '^[0-9]\+$')
        [ -n "${page_offset}" ] || page_offset=0
        local indent_width=$(echo "$3" | grep '^[0-9]\+$')
        [ -n "${indent_width}" ] || indent_width=4
        local delimiter=" | "
        local counter=0

        echo "#define OFFSET      ${page_offset}"
        echo "#define INDENT      ${indent_width}"
        echo ""

        if [ "${input_pdf}" = "" ]; then
            local sec_indent="$(printf '%*s' ${indent_width})"
            local subsec_indent="$(printf '%*s' $((${indent_width} * 2)))"

            counter=$((${counter} + 1))

            echo "Cover${delimiter}$((${counter} - ${page_offset}))"
            counter=$((${counter} + 2))

            echo "Contents${delimiter}$((${counter} - ${page_offset}))"
            counter=$((${counter} + 3))

            for i in $(seq 3)
            do
                echo "Chapter ${i}${delimiter}$((${counter} - ${page_offset}))"

                for j in $(seq ${i})
                do
                    echo "${sec_indent}Section ${i}.${j}${delimiter}$((${counter} - ${page_offset}))"

                    for k in $(seq ${j})
                    do
                        echo "${subsec_indent}Subsection ${i}.${j}.${k}${delimiter}$((${counter} - ${page_offset}))"
                        counter=$((${counter} + 5))
                    done
                done
            done

            echo "# More items with the same format as above ..."

            return 0
        fi # if [ "${input_pdf}" = "" ]: Show a bookmark example

        local invalid_indent="\r"
        local indent_spaces="${invalid_indent}"
        local title=""
        local page_num=0

        pdftk "${input_pdf}" dump_data_utf8 | grep -a '^Bookmark' | while IFS= read -r line
        do
            counter=$((${counter} + 1))
            tag="${line%%:*}"

            [ "${tag}" != "BookmarkBegin" ] || continue

            line="${line#*:}"
            [ "${line:0:1}" != " " ] || line="${line:1}"

            if [ "${tag}" = "BookmarkTitle" ]; then
                title="${line}"
            elif [ "${tag}" = "BookmarkLevel" ]; then
                indent_spaces="$(printf '%*s' $((${indent_width} * $((${line} - 1)))))"
            elif [ "${tag}" = "BookmarkPageNumber" ]; then
                page_num=$((${line} - ${page_offset}))
            else
                echo "*** Line ${counter} unrecognized: ${line}" >&2
                return 1
            fi

            if [ "${indent_spaces}" = "${invalid_indent}" -o "${title}" = "" -o ${page_num} -eq 0 ]; then
                continue
            fi

            echo "${indent_spaces}${title}${delimiter}${page_num}"

            indent_spaces="${invalid_indent}"
            title=""
            page_num=0
        done
    }

    pdf_bookmark_set()
    {
        [ -n "${FUNCNAME}" ] || FUNCNAME="pdf_bookmark_set"

        if [ $# -lt 3 ]; then
            (
                echo "*** Insufficient arguments!"
                echo "Usage: ${FUNCNAME} <input-pdf> <bookmark> <output-pdf>"
                echo "Example: ${FUNCNAME} my_old_pdf.pdf my_pdf_bookmark.txt my_new_pdf.pdf"
            ) >&2

            return 128
        fi

        local input_pdf="$1"
        local output_pdf="$3"
        local orig_bookmark="$2"
        local conv_bookmark="${2}.bmrk"

        if [ ! -f "${input_pdf}" ]; then
            echo "*** Input PDF file does not exist: ${input_pdf}" >&2
            return 1
        fi

        if [ ! -f "${orig_bookmark}" ]; then
            echo "*** Bookmark file does not exist: ${orig_bookmark}" >&2
            return 1
        fi

        if [ -f "${output_pdf}" ]; then
            echo "*** Output PDF file already exists: ${output_pdf}" >&2
            return 1
        fi

        local page_offset=$(grep -m 1 '^#define[[:blank:]]\+OFFSET[[:blank:]]\+[0-9]\+[[:blank:]]*$' ${orig_bookmark} | awk '{ print $3 }')
        [ -n "${page_offset}" ] || page_offset=0
        local indent_width=$(grep -m 1 '^#define[[:blank:]]\+INDENT[[:blank:]]\+[0-9]\+[[:blank:]]*$' ${orig_bookmark} | awk '{ print $3 }')
        [ -n "${indent_width}" ] || indent_width=4
        local indent_spaces=$(printf "%*s" ${indent_width})

        # FIXME: This is expected to preserve previous settings for page scaling, but it doesn't work!
        pdftk "${input_pdf}" dump_data_utf8 | grep -a -v '^Bookmark' > "${conv_bookmark}"

        sed -ne '/^#/d' -ne '/[：:@|][ \t]*[-+]*[0-9]\+[ \t]*$/p' "${orig_bookmark}" \
            | sed -e 's/[：:@|][ \t]*\([-+]*[0-9]\+\)[ \t]*$/@\1/' -e "s/${indent_spaces}/\t/g" \
            | while IFS= read -r line
        do
            level=$(echo "${line}" | grep -o $'\t' | wc -l)
            line="${line##*$'\t'}"

            (
                echo "BookmarkBegin"
                echo "BookmarkTitle: ${line%@*}"
                echo "BookmarkLevel: $((${level} + 1))"
                echo "BookmarkPageNumber: $((${line##*@} + ${page_offset}))"
            ) >> "${conv_bookmark}"
        done

        if [ $(grep -m 1 -c '^BookmarkBegin' "${conv_bookmark}") -eq 0 ]; then
            echo "*** The format of bookmark file is invalid: ${orig_bookmark}" >&2
            return 128
        else
            # FIXME: Previous settings for page scaling might be lost!
            pdftk "${input_pdf}" update_info_utf8 "${conv_bookmark}" output "${output_pdf}" && rm "${conv_bookmark}"
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
#   01. Initial commit.
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
# >>> 2026-01-21, Man Hung-Coeng <udc577@126.com>:
#   01. Add pdf_bookmark_{get,set}().
#

