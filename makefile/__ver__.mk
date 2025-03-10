#
# Version number based on VCS (version control system).
#
# Copyright (c) 2023-2025 Man Hung-Coeng <udc577@126.com>
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

VCS ?= git

ifeq (${VCS}, git)

    # Method 1: It works, but is a little tedious.
    __DIRTY_FLAG ?= $(shell \
        [ -z "$$(git diff --name-only . 2> /dev/null | head -n 1)" ] \
        && echo "" \
        || echo "-dirty")
    #
    #VCS_VERSION ?= $(shell \
    #    git log --abbrev-commit --abbrev=12 --pretty=oneline . 2> /dev/null \
    #    | head -n 1 \
    #    | awk '{ print $$1 }')${__DIRTY_FLAG}
    ## Or:
    VCS_VERSION ?= $(shell git log --abbrev-commit --abbrev=12 --pretty=format:%h -n 1 . 2> /dev/null)${__DIRTY_FLAG}

    # Method 2: This looks fine, but fails to get the commit hash while the latest commit is tagged.
    #VCS_VERSION ?= $(shell \
    #    git describe --abbrev=12 --dirty --always 2> /dev/null \
    #    | sed 's/.*\([0-9a-z]\{12,\}\)\(\(-dirty\).\?\)/\1\2/')

    # Method 3: Almost the perfect method but it's a pity that
	# this command doesn't support subdirectory when --dirty option is specified!
    #VCS_VERSION ?= $(shell git describe --exclude="*" --abbrev=12 --dirty --always 2> /dev/null)

else ifeq (${VCS}, svn)

    __DIRTY_FLAG ?= $(shell \
        [ -z "$$(svn status 2> /dev/null | head -n 1)" ] \
        && echo "" \
        || echo "-dirty")

    VCS_VERSION ?= $(shell \
        LANG=en_US.UTF-8 LANGUAGE=en_US.EN \
        svn info 2> /dev/null \
        | grep 'Last Changed Rev' \
        | sed 's/.* \([0-9]\)/\1/')${__DIRTY_FLAG}

else
    VCS_VERSION ?= <none>
endif

__VER__ ?= ${VCS_VERSION}

ifneq ($(filter n N no NO No 0, ${EVAL_VERSION_ONCE}),)
    undefine EVAL_VERSION_ONCE
endif
ifdef EVAL_VERSION_ONCE
    export __DIRTY_FLAG VCS_VERSION __VER__
endif

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-04-07, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2023-04-09, Man Hung-Coeng <udc577@126.com>:
#   01. Add a new macro EVAL_VERSION_ONCE
#   	to control whether to do version evaluation only once.
#
# >>> 2023-06-08, Man Hung-Coeng <udc577@126.com>:
#   01. Refine the command of fetching a git commit hash.
#
# >>> 2023-06-23, Man Hung-Coeng <udc577@126.com>:
#   01. Change the default value of VCS_VERSION from 0123456789abcdef to <none>.
#   02. Remove unwanted contents of "git describe" result.
#
# >>> 2023-07-01, Man Hung-Coeng <udc577@126.com>:
#   01. Filter out the error message produced by ${VCS}
#       while the target project is not under versioning control.
#
# >>> 2023-07-15, Man Hung-Coeng <udc577@126.com>:
#   01. Add some negative values that can invalidate EVAL_VERSION_ONCE.
#
# >>> 2023-10-07, Man Hung-Coeng <udc577@126.com>:
#   01. Fix the bug of fetching a Git tag instead of the expected commit hash
#   	while the latest commit is tagged.
#
# >>> 2024-11-10, Man Hung-Coeng <udc577@126.com>:
#   01. Use the commit hash of current directory instead of the one of project.
#
# >>> 2025-03-01, Man Hung-Coeng <udc577@126.com>:
#   01. Add --name-only option to reduce the amount of "git diff" output
#   	when checking dirty status.
#

