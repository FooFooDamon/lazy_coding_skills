# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#

.PHONY: all prepare dependencies .ALWAYS_MAKE dist distclean

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

LAZY_CODING_MAKEFILES ?= $(abspath __ver__.mk c_and_cpp.mk)

override PREREQUISITE_FILES := \
    ${LAZY_CODING_URL}/raw/77d444b429e8ac0166e876834a52e97e0d3b3237/c_and_cpp/native/signal_handling.c \
    ${LAZY_CODING_URL}/raw/77d444b429e8ac0166e876834a52e97e0d3b3237/c_and_cpp/native/signal_handling.h \

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES} $(notdir ${PREREQUISITE_FILES}), -a -s ${i}) ] && echo 1 || echo 0),0)

all prepare: dependencies
	@for i in ${LAZY_CODING_MAKEFILES}; \
	do \
		mkdir -p $$(dirname $${i}); \
		[ -s $${i} ] || wget -c -O $${i} "${LAZY_CODING_URL}/raw/main/makefiles/$$(basename $${i})"; \
	done
	@for i in ${PREREQUISITE_FILES}; \
	do \
		[ -s $$(basename $${i}) ] || wget -c -O $$(basename $${i}) "$${i}"; \
	done
	@echo "~ ~ ~ Minimum preparation finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

all: dependencies

C_SRCS := $(shell find ./ -name "*.c" | grep -v '\.priv\.c$$')
CXX_SRCS := $(shell find ./ -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" | grep -v '\.priv\.[^.]\+$$')

# GOAL is a compulsory target. XXX is whatever name you like.
# If you expect a static or shared library, then GOAL should be named libXXX.a or libXXX.so.
GOAL := XXX.elf

# Just defining dependencies is enough. No linking rule needed.
${GOAL}: $(addsuffix .o, $(basename ${C_SRCS} ${CXX_SRCS}))

#
# Or define GOALS and dependencies of each items of it
# for a multi-target project like:
#

# GOALS := XXX.elf libYYY.a libZZZ.so ...

# XXX.elf: <Dependencies of XXX.elf>

# libYYY.a: <Dependencies of libYYY.a>

# libZZZ.so: <Dependencies of libZZZ.so>

C_DEFINES +=
CXX_DEFINES +=
C_INCLUDES +=
CXX_INCLUDES +=
OTHER_CFLAGS +=
OTHER_CXXFLAGS +=
C_LDFLAGS +=
CXX_LDFLAGS +=
# ...

signal_handling.o: C_DEFINES += -U__STRICT_ANSI__

# ...

include ${LAZY_CODING_MAKEFILES}

#
# FIXME: Uncomment contents below if you have a header file for version definitions.
#
#versions.h: .revision
#	${Q}touch $@
#
#.revision: .ALWAYS_MAKE
#	${Q}[ -e $@ ] || touch $@
#	${Q}[ '$(file < $@)' = '${__VER__}' ] || printf '${__VER__}' > $@

.ALWAYS_MAKE:

# FIXME: Add more rules if needed, and delete this comment line then.

endif

DEPENDENCY_DIRS := $(abspath ../3rdparty)

dependencies:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] || continue; \
		${MAKE} -C $${i} $(filter all prepare distclean, ${MAKECMDGOALS}); \
	done
