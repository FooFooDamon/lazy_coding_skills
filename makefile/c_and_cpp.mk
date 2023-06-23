#
# Basic rules for C/C++ compilation.
#
# Copyright (c) 2021-2023 Man Hung-Coeng <udc577@126.com>
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

ifeq ($(origin CC), default)
    undefine CC
endif
CC ?= ${CROSS_COMPILE}gcc

ifeq ($(origin CXX), default)
    undefine CXX
endif
CXX ?= ${CROSS_COMPILE}g++

ifeq ($(origin AR), default)
    undefine AR
endif
AR ?= ${CROSS_COMPILE}ar

ifeq ($(origin STRIP), default)
    undefine STRIP
endif
STRIP ?= ${CROSS_COMPILE}strip

RM ?= $(shell which rm)

FLAGS_WARN ?= -Wall -Werror
FLAGS_ANSI ?= -ansi -Wpedantic
FLAGS_FOR_DEBUG ?= -O0 -g -ggdb
FLAGS_FOR_RELEASE ?= -O3 -DNDEBUG

COMMON_COMPILE_FLAGS ?= -D_REENTRANT -D__VER__=\"${__VER__}\" -fPIC ${FLAGS_WARN} \
    ${FLAGS_ANSI} -Wno-variadic-macros # -fstack-protector-strong

EXTRA_COMPILE_FLAGS ?= -W -Wno-unused-parameter -Wno-missing-field-initializers -Wno-implicit-fallthrough

NDEBUG ?= y
ifneq ($(filter n N no NO No 0, ${NDEBUG}),)
    NDEBUG :=
endif
ifeq (${NDEBUG},)
    undefine NDEBUG
endif
ifdef NDEBUG
    DEBUG_FLAGS ?= ${FLAGS_FOR_RELEASE}
else
    DEBUG_FLAGS ?= ${FLAGS_FOR_DEBUG}
endif

C_STD_FLAG ?= --std=c89
CXX_STD_FLAG ?= --std=c++11

DEFAULT_CFLAGS ?= ${DEBUG_FLAGS} ${COMMON_COMPILE_FLAGS} ${C_STD_FLAG}
DEFAULT_CXXFLAGS ?= ${DEBUG_FLAGS} ${COMMON_COMPILE_FLAGS} ${CXX_STD_FLAG}

D_FLAG ?= -Wp,-MMD,$@.d

CFLAGS ?= ${D_FLAG} ${DEFAULT_CFLAGS} ${C_DEFINES} ${C_INCLUDES} ${OTHER_CFLAGS}
CXXFLAGS ?= ${D_FLAG} ${DEFAULT_CXXFLAGS} ${CXX_DEFINES} ${CXX_INCLUDES} ${OTHER_CXXFLAGS}

C_COMPILE ?= ${CC} ${CFLAGS} -c -o $@ $<
CXX_COMPILE ?= ${CXX} ${CXXFLAGS} -c -o $@ $<

STRIP_SYMBOLS ?= if [ -n "${NDEBUG}" ]; then ${STRIP} -s -v $@; fi
ifdef ALLOW_REORDER_ON_LINKING
    C_LINK ?= ${CC} -o $@ -fPIE -Wl,--start-group $^ ${C_LDFLAGS} -Wl,--end-group && ${STRIP_SYMBOLS}
    CXX_LINK ?= ${CXX} -o $@ -fPIE -Wl,--start-group $^ ${CXX_LDFLAGS} -Wl,--end-group && ${STRIP_SYMBOLS}
else
    C_LINK ?= ${CC} -o $@ -fPIE $^ ${C_LDFLAGS} && ${STRIP_SYMBOLS}
    CXX_LINK ?= ${CXX} -o $@ -fPIE $^ ${CXX_LDFLAGS} && ${STRIP_SYMBOLS}
endif

MAKE_STATIC_LIB ?= ${AR} rsv $@ $^
MAKE_SHARED_LIB ?= ${CXX} -shared -o $@ $^

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

OBJS := $(addsuffix .o, $(basename ${C_SRCS} ${CXX_SRCS}))

# Dependencies for auto-detection of header content update.
#D_FILES ?= $(foreach i, $(shell find ./ -name "*.c" -o -name "*.cpp" -o -name "*.cxx" -o -name "*.cc"), $(basename ${i}).o.d)
D_FILES ?= $(foreach i, ${OBJS}, ${i}.d)
ifneq (${D_FILES},)
    -include ${D_FILES}
endif

PARALLEL_OPTION ?= -j $(shell grep -c "processor" /proc/cpuinfo)
__cplusplus ?= 201103L

check:
	if [ -n "${C_SRCS}" ]; then \
		cppcheck --quiet --enable=all --language=c ${C_STD_FLAG} ${PARALLEL_OPTION} \
			${C_DEFINES} ${C_INCLUDES} ${C_SRCS}; \
		clang --analyze ${CFLAGS} ${C_SRCS}; \
	fi
	if [ -n "${CXX_SRCS}" ]; then \
		cppcheck --quiet --enable=all --language=c++ ${CXX_STD_FLAG} ${PARALLEL_OPTION} \
			-D__cplusplus=${__cplusplus} ${CXX_DEFINES} ${CXX_INCLUDES} ${CXX_SRCS}; \
		clang --analyze ${CXXFLAGS} ${CXX_SRCS}; \
	fi

clean:
	[ -z "${EXECS}" ] || ${RM} ${EXECS}
	[ -z "${STATIC_LIBS}" ] || ${RM} ${STATIC_LIBS}
	[ -z "${SHARED_LIBS}" ] || ${RM} ${SHARED_LIBS}
	[ -z "${OBJS}" ] || ${RM} ${OBJS}
	[ -z "${OBJS}" ] || ${RM} ${OBJS:.o=.plist}
	[ -z "${D_FILES}" ] || ${RM} ${D_FILES} check.d

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
# >>> 2021-12-21, Man Hung-Coeng:
#   01. Add ${C_DEFINES} into CFLAGS, ${CXX_DEFINES} into CXXFLAGS.
#
# >>> 2021-12-26, Man Hung-Coeng:
#   01. Remove some flags like -fstack-protector-strong and -Wl,--start-group,
#   	which may not be supported on other platforms (e.g., MinGW and OS X).
#
# >>> 2022-02-21, Man Hung-Coeng:
#   01. Add -D_REENTRANT into COMMON_COMPILE_FLAGS.
#
# >>> 2022-09-13, Man Hung-Coeng:
#   01. Add EXTRA_COMPILE_FLAGS and -DNDEBUG.
#
# >>> 2022-10-25, Man Hung-Coeng:
#   01. Add ARFLAGS, and modify AR.
#
# >>> 2023-04-08, Man Hung-Coeng:
#   01. Remove definition __VER__ because it exists in another file __ver__.mk.
#   02. Rename this file to c_and_cpp.mk.
#
# >>> 2023-04-16, Man Hung-Coeng:
#   01. Add dependencies for auto-detection of header content update.
#
# >>> 2023-06-22, Man Hung-Coeng:
#   01. Add ${CROSS_COMPILE} prefix to values of CC, CXX, AR and STRIP.
#
# >>> 2023-06-23, Man Hung-Coeng:
#   01. Judge CC, CXX, AR and STRIP more precisely.
#   02. Remove ARFLAGS.
#   03. Add variable RM, FLAGS_*, *_STD_FLAG and MAKE_*_LIB.
#   04. Add target check and clean.
#   05. Define NDEBUG, and change the way of using it.
#   06. Enhance C_LINK and CXX_LINK.
#

