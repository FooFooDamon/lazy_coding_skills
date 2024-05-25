#!/bin/bash

#
# Useful aliases.
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

if [ -n "${LAZY_CODING_HOME}" ]; then
    alias cp="time cp"
    alias diff="diff --color=auto"
    alias dsk="cd ${HOME}/桌面 2> /dev/null || cd ${HOME}/Desktop"
    alias find="time find -L"
    alias html2pdf=wkhtmltopdf
    alias html2pic=wkhtmltoimage
    alias lc_reload=". ${LAZY_CODING_HOME}/scripts/__import__.sh"
    alias less='less -N'
    alias L="cd ${LAZY_CODING_HOME}"
    alias make="time make"
    alias mv="time mv"
    alias nfsmount='${SUDO} mount -t nfs -o nolock,soft'$([ -n "$(echo $NFS_RO | grep -i -v '0\|false\|no\|n')" ] && echo ",ro" || echo "")
    alias pst="ps -eLo uid,pid,ppid,lwp,psr,c,stime,tname,time,args" # Means displaying [t]hread info while executing ps.
    alias report_last_op_status='[ $? -eq 0 ] && play_normal_exit_audio -T || play_abnormal_exit_audio -T'
    alias report_last_op_status_once='[ $? -eq 0 ] && play_normal_exit_audio_once -T || play_abnormal_exit_audio_once -T'
    alias rm=safer-rm.sh
    alias startx="printf '\\e[0;33mstartx should not be used when you have entered a graphic desktop\\e[0m\n'"
    alias tailf="tail --follow=name"
    alias tl="[ -e ${HOME}/logs ] || mkdir ${HOME}/logs; script -f ${HOME}/logs/terminal_log_\`date +%Y-%m-%d_%H_%M_%S\`.txt"
    alias valgrind="valgrind --tool=memcheck --leak-check=full --track-origins=yes --show-reachable=yes --log-file=valgrind.log"
fi

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-02-12, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2023-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Add html2pdf and html2pic.
#
# >>> 2023-09-07, Man Hung-Coeng <udc577@126.com>:
#   01. Add report_last_op_status*.
#
# >>> 2023-10-07, Man Hung-Coeng <udc577@126.com>:
#   01. Change the alias of diff from "colordiff" to "diff --color=auto".
#
# >>> 2024-02-21, Man Hung-Coeng <udc577@126.com>:
#   01. Add "less".
#
# >>> 2024-03-26, Man Hung-Coeng <udc577@126.com>:
#   01. Add cp, find, mv and nfsmount.
#
# >>> 2024-05-25, Man Hung-Coeng <udc577@126.com>:
#   01. Add read-only support for nfsmount.
#

