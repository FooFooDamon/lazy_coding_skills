#
# Version number based on VCS(version control system).
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

VCS ?= git

ifeq (${VCS}, git)

    #__DIRTY_FLAG ?= $(shell \
    #    [ -z "$$(git diff | head -n 1)" ] \
    #    && echo "" \
    #    || echo "-dirty")

    #VCS_VERSION ?= $(shell \
    #    git log --abbrev-commit --abbrev=12 --pretty=oneline \
    #    | head -n 1 \
    #    | awk '{ print $$1 }')${__DIRTY_FLAG}
	#
	#VCS_VERSION ?= $(shell git log --abbrev-commit --abbrev=12 --pretty=format:%h -n 1)${__DIRTY_FLAG}

    VCS_VERSION ?= $(shell \
        git describe --abbrev=12 --dirty --always \
        | sed 's/.*\([0-9a-z]\{12,\}\)\(\(-dirty\)*\)/\1\2/')

else ifeq (${VCS}, svn)

    __DIRTY_FLAG ?= $(shell \
        [ -z "$$(svn status | head -n 1)" ] \
        && echo "" \
        || echo "-dirty")

    VCS_VERSION ?= $(shell \
        LANG=en_US.UTF-8 LANGUAGE=en_US.EN \
        svn info \
        | grep 'Last Changed Rev' \
        | sed 's/.* \([0-9]\)/\1/')${__DIRTY_FLAG}

else
    VCS_VERSION ?= <none>
endif

__VER__ ?= ${VCS_VERSION}

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

