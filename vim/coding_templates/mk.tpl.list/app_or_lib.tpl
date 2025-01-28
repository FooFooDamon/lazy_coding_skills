# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#

.PHONY: all prepare dependencies

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

export LAZY_CODING_MAKEFILES ?= $(abspath __ver__.mk c_and_cpp.mk)

override undefine PREREQUISITE_FILES
PREREQUISITE_FILES := ${LAZY_CODING_URL}/raw/77d444b429e8ac0166e876834a52e97e0d3b3237/c_and_cpp/native/signal_handling.c \
    ${LAZY_CODING_URL}/raw/77d444b429e8ac0166e876834a52e97e0d3b3237/c_and_cpp/native/signal_handling.h \

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES} $(notdir ${PREREQUISITE_FILES}), -a -s ${i}) ] && echo 1 || echo 0),0)

all prepare: dependencies
	@for i in ${LAZY_CODING_MAKEFILES}; \
	do \
		mkdir -p $$(dirname $${i}); \
		[ -s $${i} ] || wget -c -O $${i} "${LAZY_CODING_URL}/raw/main/makefile/$$(basename $${i})"; \
	done
	@for i in ${PREREQUISITE_FILES}; \
	do \
		[ -s $$(basename $${i}) ] || wget -c -O $$(basename $${i}) "$${i}"; \
	done
	@echo "~ ~ ~ Minimum preparation finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

all: dependencies

export EVAL_VERSION_ONCE ?= N
export NO_PRIV_STUFF := $(strip $(filter-out n N no NO No 0, ${NO_PRIV_STUFF}))

C_SRCS := $(shell find ./ -name "*.c" | grep -v '\.priv\.c$$')
ifeq (${NO_PRIV_STUFF},)
    C_SRCS := $(foreach i, $(filter-out %.priv.c, ${C_SRCS}), \
        $(if $(wildcard $(basename ${i}).priv.c), $(basename ${i}).priv.c, ${i}) \
    )
endif

CXX_SRCS := $(shell find ./ -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" | grep -v '\.priv\.[^.]\+$$')
ifeq (${NO_PRIV_STUFF},)
    CXX_SRCS := $(foreach i, $(filter-out $(addprefix %.priv, $(suffix ${CXX_SRCS})), ${CXX_SRCS}), \
        $(if $(wildcard $(basename ${i}).priv$(suffix ${i})), $(basename ${i}).priv$(suffix ${i}), ${i}) \
    )
endif

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

signal_handling.o: C_DEFINES += -U__STRICT_ANSI__

# ...

include ${LAZY_CODING_MAKEFILES}

# FIXME: Add more rules if needed, and delete this comment line then.

endif

export DEPENDENCY_DIRS ?= $(abspath ../3rdparty)

dependencies:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] && ${MAKE} $(filter all prepare, ${MAKECMDGOALS}) -C $${i} || true; \
	done
