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

define reassign_if_default
    ifeq ($(origin $1), default)
        $(eval undefine $1)
    endif
    $1 ?= $2
endef

$(eval $(call reassign_if_default, CC, ${CROSS_COMPILE}gcc))
$(eval $(call reassign_if_default, CXX, ${CROSS_COMPILE}g++))
$(eval $(call reassign_if_default, AR, ${CROSS_COMPILE}ar))
$(eval $(call reassign_if_default, STRIP, ${CROSS_COMPILE}strip))
RM ?= $(shell which rm)

ifneq ($(filter n N no NO No 0, ${__STRICT__}),)
    override __STRICT__ :=
endif

# Q is short for "quiet".
Q := $(if $(strip $(filter-out n N no NO No 0, ${V} ${VERBOSE})),,@)

NDEBUG ?= y
ifneq ($(filter n N no NO No 0, ${NDEBUG}),)
    override NDEBUG :=
endif

C_STD ?= c11
CXX_STD ?= c++11

FLAGS_WARN ?= -Wall -Wextra $(if ${__STRICT__},-Werror) -Wno-unused-parameter \
    -Wno-variadic-macros # -Wno-missing-field-initializers -Wno-implicit-fallthrough
FLAGS_ANSI ?= -ansi -Wpedantic
FLAGS_FOR_DEBUG ?= -O0 -g -ggdb
FLAGS_FOR_RELEASE ?= -O3 -DNDEBUG

DEBUG_FLAGS ?= $(if ${NDEBUG},${FLAGS_FOR_RELEASE},${FLAGS_FOR_DEBUG})

COMMON_COMPILE_FLAGS ?= -D$(shell echo __ARCH_${ARCH}__ | tr 'a-z' 'A-Z') \
    ${DEBUG_FLAGS} -D_REENTRANT -D__VER__=\"${__VER__}\" -fPIC \
    ${FLAGS_WARN} ${FLAGS_ANSI} # -fstack-protector-strong

DEFAULT_CFLAGS ?= ${COMMON_COMPILE_FLAGS} -std=${C_STD}
DEFAULT_CXXFLAGS ?= ${COMMON_COMPILE_FLAGS} -std=${CXX_STD}

D_FLAG ?= -Wp,-MMD,$*.d

CFLAGS ?= ${DEFAULT_CFLAGS} ${C_DEFINES} ${C_INCLUDES} ${OTHER_CFLAGS}
CXXFLAGS ?= ${DEFAULT_CXXFLAGS} ${CXX_DEFINES} ${CXX_INCLUDES} ${OTHER_CXXFLAGS}

C_COMPILE ?= ${CC} ${D_FLAG} ${CFLAGS} -c -o $@ $<
CXX_COMPILE ?= ${CXX} ${D_FLAG} ${CXXFLAGS} -c -o $@ $<

STRIP_SYMBOLS ?= [ -z "${NDEBUG}" ] || ${STRIP} -s $(if ${Q},,-v) $@

ALLOW_REORDER_ON_LINKING ?= y
ifneq ($(strip $(filter-out n N no NO No 0, ${ALLOW_REORDER_ON_LINKING})),)
    C_LINK ?= ${CC} -o $@ -fPIE -Wl,--start-group $^ ${C_LDFLAGS} -Wl,--end-group
    CXX_LINK ?= ${CXX} -o $@ -fPIE -Wl,--start-group $^ ${CXX_LDFLAGS} -Wl,--end-group
else
    C_LINK ?= ${CC} -o $@ -fPIE $^ ${C_LDFLAGS}
    CXX_LINK ?= ${CXX} -o $@ -fPIE $^ ${CXX_LDFLAGS}
endif

MAKE_STATIC_LIB ?= ${AR} rs$(if ${Q},,v) $@ $^
MAKE_SHARED_LIB ?= ${CXX} -shared -o $@ $^

%.o: %.c
	$(if ${Q},@printf 'CC\t$<\n')
	${Q}${C_COMPILE}

%.o: %.cc
	$(if ${Q},@printf 'CXX\t$<\n')
	${Q}${CXX_COMPILE}

%.o: %.cpp
	$(if ${Q},@printf 'CXX\t$<\n')
	${Q}${CXX_COMPILE}

%.o: %.cxx
	$(if ${Q},@printf 'CXX\t$<\n')
	${Q}${CXX_COMPILE}

ifeq ($(strip ${GOAL} ${GOALS}),)
    $(error Neither GOAL nor GOALS variable is defined and non-empty)
endif

ifndef C_SRCS
    $(warning Guessing C source files ...)
    export C_SRCS := $(sort $(shell \
        ${MAKE} ${GOAL} ${GOALS} C_SRCS=_ CXX_SRCS=$(if ${CXX_SRCS},'${CXX_SRCS}',_) --dry-run --always-make \
        | grep '^${CC} ' | grep " -o [\"']\?[^ ]\+\.o" \
        | sed "s/.*[ ]\+[\"']\?\([^ ]\+\.c\)[\"']\?[ ]*.*/\1/"))
endif

ifndef CXX_SRCS
    $(warning Guessing CXX source files ...)
    export CXX_SRCS := $(sort $(shell \
        ${MAKE} ${GOAL} ${GOALS} C_SRCS=$(if ${C_SRCS},'${C_SRCS}',_) CXX_SRCS=_ --dry-run --always-make \
        | grep '^${CXX} ' | grep " -o [\"']\?[^ ]\+\.o" \
        | sed "s/.*[ ]\+[\"']\?\([^ ]\+\.\(cc\|cpp\|cxx\)\)[\"']\?[ ]*.*/\1/"))
endif

ifeq ($(strip ${C_SRCS} ${CXX_SRCS}),)
    $(error Can not find any C or CXX source file)
endif

# Dependencies for auto-detection of header content update.
D_FILES ?= ${C_SRCS:.c=.d} $(foreach i, $(basename ${CXX_SRCS}), ${i}.d)
ifneq ($(strip ${D_FILES}),)
    #-include ${D_FILES} # Arguments might overflow if item count of ${D_FILES} is too large.
    $(foreach i, ${D_FILES}, $(eval -include ${i}))
endif

ARCH_LIST ?= arm avr host mips powerpc x86
# NOTE: The "host" is the architecture of host computer CPU, which is usually x86.
ARCH ?= host
CROSS_COMPILE_FOR_arm ?= arm-linux-gnueabihf-
CROSS_COMPILE_FOR_avr ?= avr-
CROSS_COMPILE_FOR_mips ?= mips-linux-gnu-
CROSS_COMPILE_FOR_powerpc ?= powerpc-linux-gnu-
PARALLEL_OPTION ?= -j $(shell grep -c "processor" /proc/cpuinfo)
__cplusplus ?= 201103L

.PHONY: all $(foreach i, ${ARCH_LIST}, ${i}-release ${i}-debug) check clean

all: ${ARCH}-release

${GOAL} ${GOALS}: %:
	${Q}if [ -n "$$(echo '$@' | grep '^lib.*\.a.*')" ]; then \
		$(if ${Q},printf 'AR\t$@\n';) \
		${MAKE_STATIC_LIB}; \
	elif [ -n "$$(echo '$@' | grep '^lib.*\.so.*')" ]; then \
		$(if ${Q},printf 'LD\t$@\n';) \
		${MAKE_SHARED_LIB}; \
	else \
		$(if ${Q},printf 'LD\t$@\n';) \
		${CXX_LINK}; \
		[ -z "${NDEBUG}" ] || $(if ${Q},printf 'STRIP\t$@\n',:); \
		${STRIP_SYMBOLS}; \
	fi

$(foreach i, ${ARCH_LIST}, ${i}-release): %:
	${Q}${MAKE} ${GOAL} ${GOALS} ${PARALLEL_OPTION} \
		ARCH=${@:-release=} CROSS_COMPILE=${CROSS_COMPILE_FOR_${@:-release=}}

$(foreach i, ${ARCH_LIST}, ${i}-debug): %:
	${Q}${MAKE} ${GOAL} ${GOALS} ${PARALLEL_OPTION} \
		ARCH=${@:-debug=} CROSS_COMPILE=${CROSS_COMPILE_FOR_${@:-debug=}} NDEBUG=0

check:
	$(if ${Q},@printf '>>> CHECK: Begin.\n')
	$(if ${Q},@printf '>>> CHECK: Patience ...\n')
	${Q}if [ -n "$(word 1, ${C_SRCS})" ]; then \
		cppcheck --quiet --enable=all --language=c --std=${C_STD} ${PARALLEL_OPTION} \
			${C_DEFINES} ${C_INCLUDES} ${C_SRCS}; \
		clang --analyze ${CFLAGS} ${C_SRCS}; \
	fi
	${Q}if [ -n "$(word 1, ${CXX_SRCS})" ]; then \
		cppcheck --quiet --enable=all --language=c++ --std=${CXX_STD} ${PARALLEL_OPTION} \
			-D__cplusplus=${__cplusplus} ${CXX_DEFINES} ${CXX_INCLUDES} ${CXX_SRCS}; \
		clang --analyze ${CXXFLAGS} ${CXX_SRCS}; \
	fi
	$(if ${Q},@printf '>>> CHECK: Done.\n')

clean:
	$(if ${Q},@printf '>>> CLEAN: Begin.\n')
	${Q}[ -z "$(word 1, ${GOAL} ${GOALS})" ] || ${RM} ${GOAL} ${GOALS}
	${Q}${RM} ${D_FILES:.d=.o}
	${Q}${RM} ${D_FILES:.d=.plist} *.plist
	${Q}${RM} ${D_FILES}
	$(if ${Q},@printf '>>> CLEAN: Done.\n')

__VARS__ := CROSS_COMPILE CC CXX AR STRIP RM __STRICT__ C_STD CXX_STD __cplusplus \
    FLAGS_WARN FLAGS_ANSI FLAGS_FOR_DEBUG FLAGS_FOR_RELEASE NDEBUG D_FLAG \
    C_DEFINES CXX_DEFINES C_INCLUDES CXX_INCLUDES OTHER_CFLAGS OTHER_CXXFLAGS \
    CFLAGS CXXFLAGS C_COMPILE CXX_COMPILE ALLOW_REORDER_ON_LINKING C_LINK CXX_LINK STRIP_SYMBOLS \
	MAKE_STATIC_LIB MAKE_SHARED_LIB GOAL GOALS C_SRCS CXX_SRCS \
    ARCH_LIST ARCH $(foreach i, ${ARCH_LIST}, CROSS_COMPILE_FOR_${i}) \
    PARALLEL_OPTION

ifeq (${Q},)
    $(info -)
    $(foreach i, ${__VARS__}, \
        $(eval \
            $(info \
                - ${i}: ${$i} \
                $(if \
                    $(filter-out CROSS_COMPILE, $(filter %_COMPILE %_LINK MAKE_%_LIB D_FLAG STRIP_SYMBOLS, ${i})), \
                    <-- Might miss "$$@" or/and "$$<" or/and "$$^" on displaying. \
                ) \
            ) \
        ) \
    )
    $(info -)
    $(info You can override any of variables above to meet your need.)
    $(info -)
else
    ifeq (${MAKELEVEL}, 0)
        $(info Run with "V=1" or "VERBOSE=1" if you're interested in compilation details.)
    endif
endif

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2021-12-11, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2021-12-12, Man Hung-Coeng <udc577@126.com>:
#   01. Rename variable BIZ_VERSION to VCS_VERSION.
#   02. Add variable AR, STRIP and __VER__.
#
# >>> 2021-12-14, Man Hung-Coeng <udc577@126.com>:
#   01. Use ${} instead of $() to reference a variable.
#   02. Add variable C_COMPILE and CXX_COMPILE.
#   03. Comment out .c-to-.o, .cc-to.o and .cpp-to-.o rules,
#       since they're built-in rules.
#   04. Add -Werror into COMMON_COMPILE_FLAGS to make compilation stricter.
#
# >>> 2021-12-21, Man Hung-Coeng <udc577@126.com>:
#   01. Add ${C_DEFINES} into CFLAGS, ${CXX_DEFINES} into CXXFLAGS.
#
# >>> 2021-12-26, Man Hung-Coeng <udc577@126.com>:
#   01. Remove some flags like -fstack-protector-strong and -Wl,--start-group,
#       which may not be supported on other platforms (e.g., MinGW and OS X).
#
# >>> 2022-02-21, Man Hung-Coeng <udc577@126.com>:
#   01. Add -D_REENTRANT into COMMON_COMPILE_FLAGS.
#
# >>> 2022-09-13, Man Hung-Coeng <udc577@126.com>:
#   01. Add EXTRA_COMPILE_FLAGS and -DNDEBUG.
#
# >>> 2022-10-25, Man Hung-Coeng <udc577@126.com>:
#   01. Add ARFLAGS, and modify AR.
#
# >>> 2023-04-08, Man Hung-Coeng <udc577@126.com>:
#   01. Remove definition __VER__ because it exists in another file __ver__.mk.
#   02. Rename this file to c_and_cpp.mk.
#
# >>> 2023-04-16, Man Hung-Coeng <udc577@126.com>:
#   01. Add dependencies for auto-detection of header content update.
#
# >>> 2023-06-22, Man Hung-Coeng <udc577@126.com>:
#   01. Add ${CROSS_COMPILE} prefix to values of CC, CXX, AR and STRIP.
#
# >>> 2023-06-23, Man Hung-Coeng <udc577@126.com>:
#   01. Judge CC, CXX, AR and STRIP more precisely.
#   02. Remove ARFLAGS.
#   03. Add variable RM, FLAGS_*, *_STD_FLAG and MAKE_*_LIB.
#   04. Add target "check" and "clean".
#   05. Define NDEBUG, and change the way of using it.
#   06. Enhance C_LINK and CXX_LINK.
#
# >>> 2023-06-24, Man Hung-Coeng <udc577@126.com>:
#   01. Remove OBJS, and guess C_SRCS and CXX_SRCS if they're not defined.
#   02. Merge EXTRA_COMPILE_FLAGS into FLAGS_WARN.
#   03. Label target "check" and "clean" as .PHONY.
#   04. Change C_STD_FLAG to C_STD, CXX_STD_FLAG to CXX_STD.
#   05. Make the condition statements of target "clean" more precise.
#
# >>> 2023-06-25, Man Hung-Coeng <udc577@126.com>:
#   01. Remove duplicate items of C_SRCS and CXX_SRCS, and make their rules
#       more robust when facing interferences of single/double quotation marks.
#   02. While clearing and undefining __STRICT__ and NDEBUG,
#       override their counterparts from command line as well.
#   03. Change the required variables from EXECS, STATIC_LIBS, SHARED_LIBS
#       to GOAL, GOALS.
#
# >>> 2023-07-01, Man Hung-Coeng <udc577@126.com>:
#   01. Use function $(eval) and $(if) to refine some logic blocks,
#       and improve robustness and readability.
#   02. Support quiet working mode and multiple architectures.
#   03. Enable parallel compilation by default.
#   04. Provide important variables printing.
#
# >>> 2023-07-13, Man Hung-Coeng <udc577@126.com>:
#   01. Change the dependency file suffix from .o.d to .d
#       and eliminate check.d which is accidentally generated on "make check".
#   02. Fix the bug of an unexpected target "_" showing up due to deduction of
#       C_SRCS and CXX_SRCS during parallel compilation.
#

