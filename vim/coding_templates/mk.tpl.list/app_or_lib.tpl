#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
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

.PHONY: all prepare

LAZY_CODING_MAKEFILES := __ver__.mk c_and_cpp.mk

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES}, -a -s ${i}) ] && echo 1 || echo 0),0)

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

all prepare:
	@for i in ${LAZY_CODING_MAKEFILES}; \
	do \
		mkdir -p $$(dirname $${i}); \
		[ -s $${i} ] || wget -c -O $${i} "${LAZY_CODING_URL}/raw/main/makefile/$$(basename $${i})"; \
	done
	@echo "~ ~ ~ Minimum preparation finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

all: prepare

EVAL_VERSION_ONCE ?= N
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

# ...

include ${LAZY_CODING_MAKEFILES}

export DEPENDENCY_DIRS ?= ../3rdparty

prepare:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] && ${MAKE} -C $${i} || true; \
	done

# FIXME: Add more rules if needed, and delete this comment line then.

endif
