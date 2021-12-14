#
# Basic rules for C/C++ compilation.
#
# Copyright (c) 2021 Man Hung-Coeng <udc577@126.com>
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
# USAGE: Re-assign some variables in your makefile if necessary,
#        then include this file at the end of it.
#

CC ?= gcc
CXX ?= g++
AR ?= ar -r -s
STRIP ?= strip

VCS ?= git

ifeq (${VCS}, git)

    __DIRTY_FLAG = $(shell \
        [ -z "$$(git diff | head -n 1)" ] \
        && echo "" \
        || echo ".dirty")

    VCS_VERSION ?= $(shell \
        git log --abbrev-commit --abbrev=12 --pretty=oneline \
        | head -n 1 \
        | awk '{ print $$1 }')${__DIRTY_FLAG}

else ifeq (${VCS}, svn)

    __DIRTY_FLAG = $(shell \
        [ -z "$$(svn status | head -n 1)" ] \
        && echo "" \
        || echo ".dirty")

    VCS_VERSION ?= $(shell \
        LANG=en_US.UTF-8 LANGUAGE=en_US.EN \
        svn info \
        | grep 'Last Changed Rev' \
        | sed 's/.* \([0-9]\)/\1/')${__DIRTY_FLAG}

else
    VCS_VERSION ?= 0123456789abcdef
endif

__VER__ ?= ${VCS_VERSION}

COMMON_COMPILE_FLAGS ?= -D__VER__=\"${__VER__}\" -fPIC -Wall -Werror \
    -ansi -Wpedantic -Wno-variadic-macros -fstack-protector-strong

ifeq (${NDEBUG}, 1)
    DEBUG_FLAGS ?= -O3
else
    DEBUG_FLAGS ?= -O0 -g -ggdb
endif

DEFAULT_CFLAGS ?= ${COMMON_COMPILE_FLAGS} ${DEBUG_FLAGS}
DEFAULT_CXXFLAGS ?= ${COMMON_COMPILE_FLAGS} ${DEBUG_FLAGS}

CFLAGS ?= ${DEFAULT_CFLAGS} ${C_INCLUDES} ${OTHER_CFLAGS}
CXXFLAGS ?= ${DEFAULT_CXXFLAGS} ${CXX_INCLUDES} ${OTHER_CXXFLAGS}

C_COMPILE ?= ${CC} ${CFLAGS} -c -o $@ $<
CXX_COMPILE ?= ${CXX} ${CXXFLAGS} -c -o $@ $<

C_LINK ?= ${CC} -o $@ -fPIE -Wl,--start-group $^ ${C_LDFLAGS} -Wl,--end-group
CXX_LINK ?= ${CXX} -o $@ -fPIE -Wl,--start-group $^ ${CXX_LDFLAGS} -Wl,--end-group

# This is a built-in rule and needn't be written out explicitly.
#%.o: %.c
#	${C_COMPILE}

# Another built-in rule. Also applicable to .C and .cc.
#%.o: %.cpp
#	${CXX_COMPILE}

# Some developers like .cxx suffix for C++,
# note that there's no built-in rule for it.
%.o: %.cxx
	${CXX_COMPILE}

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2021-12-11, Man Hung-Coeng:
#   01. Create.
#
# >>> 2021-12-12, Man Hung-Coeng:
#   01. Rename variable BIZ_VERSION to VCS_VERSION.
#   02. Add variable AR, STRIP and __VER__.
#
# >>> 2021-12-14, Man Hung-Coeng:
#   01. Use ${} instead of $() to reference a variable.
#   02. Add variable C_COMPILE and CXX_COMPILE.
#   03. Comment out .c-to-.o, .cc-to.o and .cpp-to-.o rules,
#       since they're built-in rules.
#   04. Add -Werror into COMMON_COMPILE_FLAGS to make compilation stricter.
#

