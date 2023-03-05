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

DATETIME_CMD="date +%Y-%m-%d_%H:%M:%S.%N"

usage()
{
    printf "\n$(basename $0) - Dump M3U8 video stream.\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...] <URL, or local playlist> [resulting video file]\n"
    printf "  -h, --help     Show this help message.\n"
    printf "  -v, --version  Show version info.\n"
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) playlist.m3u8\n"
    printf "  2. $(basename $0) playlist.m3u8 test_video_1.mp4\n"
    printf "  3. $(basename $0) 'http://www.xxx.com/index.m3u8'\n"
    printf "  4. $(basename $0) 'http://www.xxx.com/index.m3u8' 'test video 2'.mkv\n"
    printf "  5. LCS_HTTP_USER_AGENT='Mozilla/5.0 (X11; Linux x86_64)' $(basename $0) 'http://www.xxx.com/index.m3u8'\n"
    printf "\nNOTES:\n"
    printf "  1. This script supports breakpoint-resuming.\n"
    printf "     So, feel free to abort and restart at any time.\n"
    printf "  2. Sometimes you need to specify the HTTP user agent.\n"
    printf "     See example 5.\n"
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
        continue
    fi
done

precheck()
{
    local _pass=1

    for i in ffmpeg parallel wget
    do
        if [ -z "$(which ${i})" ]; then
            _pass=0
            printE "*** Can not find \"${i}\" command, please install it first!"
        fi
    done

    [ ${_pass} -eq 1 ] && return 0 || return 1
}

precheck || exit 1

if [ -z "$1" ]; then
    printE "*** Please specify an URL, or a local .m3u8 file!"
    usage
    exit 1
fi

shopt -s expand_aliases

#
# Prepare.
#
[ -n "${LCS_RETRIES}" ] || LCS_RETRIES=5
[ -n "${LCS_HTTP_USER_AGENT}" ] || LCS_HTTP_USER_AGENT="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.122 Safari/537.36"
#export WGET="wget -t ${LCS_RETRIES} --no-check-certificate --user-agent=\"${LCS_HTTP_USER_AGENT}\"" # Does not work.
alias WGET="wget -t ${LCS_RETRIES} --no-check-certificate --user-agent=\"${LCS_HTTP_USER_AGENT}\""
export REMOTE_PLAYLIST=remote_playlist.lc.m3u8
export LOCAL_PLAYLIST=local_playlist.lc.m3u8
export RESULT_FILE=result.log
if [ -f "$1" ]; then
    export url="$(realpath "$1")"
    export md5=$(md5sum "${url}" | awk '{ print $1 }')
else
    export url="$1"
    export md5=$(printf "${url}" | md5sum | awk '{ print $1 }')
fi
[ -n "$2" ] && video_name="$2" || video_name="${md5}.mp4"

mkdir -p lc-m3u8.${md5} && cd lc-m3u8.${md5} || exit 1

#
# Download playlist if needed.
#
touch ${REMOTE_PLAYLIST} ${RESULT_FILE}
if [ -f "${url}" ]; then
    cp "${url}" ${REMOTE_PLAYLIST}
    export url_prefix=""
else
    [ $(wc -l ${REMOTE_PLAYLIST} | awk '{ print $1 }') -gt 0 ] || WGET "${url}" -O ${REMOTE_PLAYLIST} || exit 1
    export url_prefix="$(dirname "${url}")"
fi
if [ $(grep -c "\.m3u" ${REMOTE_PLAYLIST}) -gt 0 ]; then
    echo "[W] Secondary playlist/index:" >&2
    cat ${REMOTE_PLAYLIST}
    [ $(grep -c "^http:\|^https:\|^rtp:" ${REMOTE_PLAYLIST}) -eq 0 ] \
        && url="${url_prefix}/$(grep "\.m3u" ${REMOTE_PLAYLIST} | tail -n 1)" \
        || url="$(grep "\.m3u" ${REMOTE_PLAYLIST} | tail -n 1)"
    WGET "${url}" -O ${REMOTE_PLAYLIST} || exit 1
    export url_prefix="$(dirname "${url}")"
fi

make_url()
{
    if [ $(echo "$1" | grep -c "^http:\|^https:\|^ftp:\|^rtp:") -eq 0 ]; then
        [ "${1:0:1}" = "/" ] && echo "$(echo "${url_prefix}" | sed 's/\(.*:\/\/[^\/]*\)\/.*/\1/')/$1" || echo "${url_prefix}/$1"
    else
        echo "$1"
    fi
}

dumping_task()
{
    set -x

    local _fragment="$(basename "$1")"

    if [ $(grep -c "$(echo "${_fragment}" | sed "s/\./\\\./g")" ${RESULT_FILE}) -eq 0 ]; then
        WGET -c "$(make_url "$1")" -O "${_fragment}" || exit 1
        echo "${_fragment}" >> ${RESULT_FILE}
    fi

    set +x
}

#
# Download the secret key if any.
#
if [ $(grep -c "EXT-X-KEY:.*URI" ${REMOTE_PLAYLIST}) -gt 0 ]; then
    secret_key_url="$(grep "EXT-X-KEY:.*URI" ${REMOTE_PLAYLIST} | sed -e "s/.*URI=\([^,]*\)[,]*.*/\1/g" -e "s/\"//g")"
    secret_key_url="$(make_url "${secret_key_url}")"
    touch key.key
    [ $(wc -c key.key | awk '{ print $1 }') -gt 0 ] || WGET "${secret_key_url}" -O key.key || exit 1
fi

#
# Download video fragments.
#
export -f make_url
export -f dumping_task
set -x
grep -v "^#" ${REMOTE_PLAYLIST} | parallel -j $(grep -c "processor" /proc/cpuinfo) dumping_task "{}" || exit 1
#set +x

# Transform to local playlist.
sed -e "/^[^#]/s/.*\/\(.*\)/\1/g" -e "s/\(EXT-X-KEY:.*,URI=\)[^,]*\([,]*.*\)/\1\"key.key\"\2/g" ${REMOTE_PLAYLIST} > ${LOCAL_PLAYLIST}

# Splice fragments together into a video.
ffmpeg -allowed_extensions ALL -protocol_whitelist "file,http,https,crypto,tcp,tls" \
    -i ${LOCAL_PLAYLIST} -c copy ../"${video_name}"

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2023-02-12, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> V1.0.1|2023-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Add 3 new functions: printW(), printE() and eexit().
#
# >>> V1.0.2|2023-03-05, Man Hung-Coeng <udc577@126.com>:
#   01. Change "lz" to "lc", "lz_video_stream" to "lc-m3u8".
#

