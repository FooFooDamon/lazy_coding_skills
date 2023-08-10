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
    printf "\n$(basename $0) - Update local git repositories.\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...] [default git root directory]\n"
    printf "  -h, --help     Show this help message.\n"
    printf "  -v, --version  Show version info.\n"
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) /home/xxx/git\n"
    printf "  2. $(basename $0)\n"
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

SIGNAL_ITEMS=(INT QUIT)
for i in ${SIGNAL_ITEMS[@]}
do
    trap "handle_sig${i}" ${i}
done

DEFAULT_GIT_ROOT=""
for i in "$@"
do
    if [ "${i}" = "-h" ] || [ "${i}" = "--help" ]; then
        usage
        exit 0
    elif [ "${i}" = "-v" ] || [ "${i}" = "--version" ]; then
        version
        exit 0
    else
        DEFAULT_GIT_ROOT="${i}"
    fi
done
[ -n "${DEFAULT_GIT_ROOT}" ] || DEFAULT_GIT_ROOT="${HOME}/git"
echo "Default git root directory: ${DEFAULT_GIT_ROOT}"

[ -e "${HOME}/etc" ] || mkdir "${HOME}/etc" || exit 1

[ -n "${REPO_CONFIG}" ] || REPO_CONFIG="${HOME}/etc/git_repositories.txt"
if [ ! -f "${REPO_CONFIG}" ]; then
    touch "${REPO_CONFIG}" || exit 1
    echo "lazy_coding_skills ::: https://github.com/FooFooDamon/lazy_coding_skills.git" > "${REPO_CONFIG}"
    echo "copy_of_lcs ::: https://github.com/FooFooDamon/lazy_coding_skills.git ::: ${HOME}" >> "${REPO_CONFIG}"
    echo "# inactive_repo ::: https://github.com/FooFooDamon/lazy_coding_skills.git" >> "${REPO_CONFIG}"
fi
echo "Read Git repositories from: ${REPO_CONFIG}."

while read i
do
    [ $(echo "${i}" | grep -c '^[ \t]*$') -eq 0 ] || continue
    [ $(echo "${i}" | grep -c '^[ \t]*#') -eq 0 ] || continue

    repo_name=$(echo ${i} | awk -F ":::" '{ print $1 }')
    repo_url=$(echo ${i} | awk -F ":::" '{ print $2 }')
    git_root=$(echo ${i} | awk -F ":::" '{ print $3 }')
    [ $(echo "${git_root}" | grep '^[ \t]*$' -c) -eq 0 ] || git_root="${DEFAULT_GIT_ROOT}"

    if [ -e ${git_root}/${repo_name} ]; then
        printW "Updating ${git_root}/${repo_name} ..."
        cd ${git_root}/${repo_name} && git status && git pull
    else
        printW "Creating new Git repo: ${git_root}/${repo_name} ..."
        cd ${git_root} && git clone ${repo_url} ${repo_name}
    fi
done < "${REPO_CONFIG}"

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2023-02-11, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> V1.0.1|2023-02-12, Man Hung-Coeng <udc577@126.com>:
#   01. Automatically create ~/etc directory if it does not exist.
#
# >>> V1.0.2|2023-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Add 3 new functions: printW(), printE() and eexit().
#
# >>> V1.0.3|2023-05-29, Man Hung-Coeng <udc577@126.com>:
#   01. Fix a redirection error in creating the configuration file.
#
# >>> V1.0.4|2023-08-10, Man Hung-Coeng <udc577@126.com>:
#   01. Allow setting REPO_CONFIG.
#

