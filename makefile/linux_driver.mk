#
# Makefile template for Linux driver.
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

#
# >>>>>>>> USAGE <<<<<<<<
#
# Including this file in your own makefile and doing some customizations
# is preferable to modifying this file directly.
#
# Example 1 (Simplest; Usually for tiny tests or practices):
# (1) Assume you have multiple tests like:
#         /path/to/test/root/directory
#             |-- test1
#             |       `-- test1.c
#             |-- test2
#             |       |-- test2.c
#             |       `-- test2_app.c
#             .		.
#             .		.
#             .		.
#             `-- testN
#                     `-- testN.c
# (2) Put this file in ${HOME}/src directory, or anywhere you want.
# (3) If you're developing drivers for host computer,
#     you don't even need to write a makefile,
# 	  just making a symbolic link is enough, take test1 as example:
#         $ ln -s ${HOME}/src/linux_driver.mk /path/to/test1/Makefile
# (3') If you're developing drivers for another platform, e.g. ARM,
# 	  one more step needs to be done first:
# 	  Download Linux kernel source code and make a symbolic link:
#         $ ln -s /path/to/kernel/source/code ${HOME}/src/linux
# 	  Then, make a symbolic link for each test case as step (3).
#
# Example 2 (A bit complicated; Usually for formal projects):
# (1) Assume you have a formal project like:
#         /path/to/project/source/code/directory
#             |-- common
#             |       `-- __ver__.mk # your definition of __VER__
#             |-- app
#             |-- kernel # linux kernel source code
#             `-- drivers
#                     |-- driver1
#                     |       |-- driver1_main.c
#                     |       |-- driver1_utils.c
#                     |       |-- driver1_app_main.c
#                     |       |-- driver1_app_utils.c
#                     |       `-- Makefile
#                     |-- driver2 # containing stuff similar to driver1
#                     .		.
#                     .		.
#                     .		.
#                     `-- driverN # containing stuff similar to driver1
# (2) Put this file in the common directory.
# (3) Edit the makefile of each driver according to the actual situation,
#     let's take driver1 as example, its makefile may be like:
#         export CROSS_KERNEL_DIR ?= ${PWD}/../../kernel
#         export DRVNAME ?= driver1
#         export ${DRVNAME}-objs ?= driver1_main.o driver1_utils.o
#         export APP_NAME ?= driver1_app
#         export APP_OBJS ?= driver1_app_main.o driver1_app_utils.o
#         # Other settings if needed: APP_DEFINES, APP_INCLUDES, OTHER_APP_CFLAGS, etc.
#         include ${PWD}/../../common/__ver__.mk
#         include ${PWD}/../../common/linux_driver.mk
#
# Supported compilation commands:
#   make
#   make clean
#   make {arm|host|x86|...}-release
#   make {arm|host|x86|...}-debug
#   make clean-{arm|host|x86|...}
#   make ${DRVNAME}.ko [ARCH={arm|host|x86|...}]
#   make clean-${DRVNAME}.ko [ARCH={arm|host|x86|...}]
#   make ${APP_NAME}.elf [ARCH={arm|host|x86|...}]
#   make clean-${APP_NAME}.elf
#

define reassign_if_default
    ifeq ($(origin $1), default)
        $(eval undefine $1)
    endif
    $1 ?= $2
endef

#=======================
# Global settings.
#=======================

ARCH_LIST ?= arm avr32 host mips powerpc x86
export HOST_ARCH ?= $(shell uname -m | sed 's/\(.*\)[-_]\(32\|64\)\?$$/\1/')
# NOTE: The "host" is the architecture of host computer CPU, which is usually x86.
ARCH ?= host
ifeq (${ARCH},host)
    override ARCH := ${HOST_ARCH}
endif
CROSS_COMPILE_FOR_arm ?= arm-linux-gnueabihf-
CROSS_COMPILE_FOR_avr32 ?= avr-
CROSS_COMPILE_FOR_mips ?= mips-linux-gnu-
CROSS_COMPILE_FOR_powerpc ?= powerpc-linux-gnu-
CROSS_COMPILE ?= ${CROSS_COMPILE_FOR_${ARCH}}

ifeq (${KERNELRELEASE},)
$(eval $(call reassign_if_default, CC, ${CROSS_COMPILE}gcc))
endif

$(eval $(call reassign_if_default, STRIP, ${CROSS_COMPILE}strip))

RM ?= $(shell which rm) -f

# Q is short for "quiet".
Q := $(if $(strip $(filter-out n N no NO No 0, ${V} ${VERBOSE})),,@)

NDEBUG ?= y
ifneq ($(filter n N no NO No 0, ${NDEBUG}),)
    override NDEBUG :=
endif

export SRCS ?= $(shell find ./ -name "*.c" | grep -v '\.mod\.c$$')

#=======================
# For device driver.
#=======================

HOST_KERNEL_DIR ?= /lib/modules/`uname -r`/build
CROSS_KERNEL_DIR ?= ${HOME}/src/linux
ROOT ?= $(if $(filter host ${HOST_ARCH}, ${ARCH}),${HOST_KERNEL_DIR},${CROSS_KERNEL_DIR})

ifeq (${DRVNAME},) # Define it explicitly in your own makefile if there're multiple source files for this driver.
    export DRVNAME := $(notdir $(word 1, $(filter-out %_app, ${SRCS:.c=})))
    ifeq (${DRVNAME},)
        $(error DRVNAME not specified and not deductive)
    endif
endif

ifneq (${KERNELRELEASE},)

obj-m := ${DRVNAME}.o

# CFLAGS is not permitted here, otherwise an error will be triggered with a message below:
# *** CFLAGS was changed in "...". Fix it to use ccflags-y.
ccflags-y += -D__VER__=\"${__VER__}\" # Define the version number in another makefile, or just ignore it.
ifeq (${NDEBUG},)
    ccflags-y += -O0 -g
endif
# NOTE 1: The reason of defining a new macro __SRC__ is that
# 		re-defining the built-in __FILE__ will cause a compilation warning.
# 		In most cases, an object file (.o) is generated from a source file,
# 		not a header file, thus using __SRC__ in an inline function within a header
# 		file is almost a mistake! One must avoid doing that!
# 		If it's unavoidable, try fixing things up in your source file like this:
# 			#undef __SRC__
# 			#define __SRC__ "relative/path/to/some_inline_functions.h"
# 			#include "some_inline_functions.h"
# 			#undef __SRC__
# 			#define __SRC__ "relative/path/to/this_source_file.c"
# NOTE 2: Defining __SRC__ as __FILE__ won't cause the problem above.
# 		However, printing or logging contents with __SRC__ may be a little more
# 		if __FILE__ is an absolute path.
USE_SRC_RELATIVE_PATH ?= 0
ifeq ($(strip $(filter n N no NO No 0, ${USE_SRC_RELATIVE_PATH})),)
    $(foreach i, $(if ${${DRVNAME}-objs},${${DRVNAME}-objs},${obj-m}), \
        $(eval ${PWD}/${i}: ccflags-y += -D__SRC__=\"${i:.o=.c}\") \
    )
endif
#$(info ^v^v^v^v^v^v^v^v [MAKELEVEL:${MAKELEVEL}]: ${ccflags-y})

endif # ifneq (${KERNELRELEASE},)

ifeq (${KERNELRELEASE},)

#=======================
# For application demo.
#=======================

ifneq ($(filter n N no NO No 0, ${__STRICT__}),)
    override __STRICT__ :=
endif

export APP_NAME ?= $(notdir $(word 1, $(filter %_app, ${SRCS:.c=})))
export APP_OBJS ?= $(if ${APP_NAME},${APP_NAME}.o)
ifeq (${NDEBUG},)
    APP_DEBUG_FLAGS ?= -O0 -g
else
    APP_DEBUG_FLAGS ?= -O2 -DNDEBUG
endif
APP_CFLAGS ?= -D_REENTRANT -D__VER__=\"${__VER__}\" -fPIC -Wall -Wextra \
    $(if ${__STRICT__},-Werror) -ansi -Wpedantic \
    -Wno-variadic-macros -Wno-unused-parameter -Wno-missing-field-initializers \
    -Wno-implicit-fallthrough ${APP_DEBUG_FLAGS} ${APP_DEFINES} ${APP_INCLUDES} \
    ${OTHER_APP_CFLAGS}

#=======================
# Targets and Rules.
#=======================

export PARALLEL_OPTION ?= -j $(shell grep -c "processor" /proc/cpuinfo)

.PHONY: all $(foreach i, ${ARCH_LIST}, ${i}-release ${i}-debug clean-${i}) \
    clean $(if ${APP_NAME}, clean-${APP_NAME}.elf) clean-${DRVNAME}.ko

all: ${PREREQUISITES} ${ARCH}-release

${DRVNAME}.ko: $(if $(wildcard ${obj-m:.o=.c}), ${obj-m:.o=.c}) ${${DRVNAME}-objs:.o=.c}
	${Q}# Also includes the generation of .o files.
	${Q}${MAKE} -C ${ROOT} M=`pwd` ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
	${Q}[ -f $@ ] || mv $$(ls *.ko | head -n 1) $@
	${Q}[ -z "${NDEBUG}" ] || $(if ${Q}, printf 'STRIP\t$@\n', :)
	${Q}[ -z "${NDEBUG}" ] || ${STRIP} -d $(if ${Q},,-v) $@

ifneq (${APP_NAME},)

${APP_NAME}.elf: ${APP_OBJS}
	$(if ${Q},@printf 'LD\t$@\n')
	${Q}${CC} -o $@ -fPIE -Wl,--start-group $^ ${APP_LDFLAGS} -Wl,--end-group
	${Q}[ -z "${NDEBUG}" ] || $(if ${Q}, printf 'STRIP\t$@\n', :)
	${Q}[ -z "${NDEBUG}" ] || ${STRIP} -s $(if ${Q},,-v) $@

endif

$(foreach i, ${ARCH_LIST}, ${i}-release): %:
	${Q}${MAKE} ${DRVNAME}.ko $(if ${APP_NAME}, ${APP_NAME}.elf) ${PARALLEL_OPTION} ARCH=${@:-release=}

$(foreach i, ${ARCH_LIST}, ${i}-debug): %:
	${Q}${MAKE} ${DRVNAME}.ko $(if ${APP_NAME}, ${APP_NAME}.elf) ${PARALLEL_OPTION} ARCH=${@:-debug=} NDEBUG=0

D_FILES := $(foreach i, ${APP_OBJS}, ${i:.o=.d})
CMD_FILES := $(shell find . -name ".*.o.cmd" | grep -v "\.mod\.o\.cmd")

# Dependencies for auto-detection of header content update.
-include ${D_FILES}
ifneq (${CMD_FILES},)
    -include ${CMD_FILES}
endif

%.o: %.c
	$(if ${Q},@printf 'CC\t$<\n')
	${Q}${CC} -Wp,-MMD,$*.d ${APP_CFLAGS} -c -o $@ $<

# TODO: Add a "check" target for code static checking.

ifneq (${APP_NAME},)

clean-${APP_NAME}.elf:
	$(if ${Q},@printf '>>> CLEAN[${APP_NAME}.elf]: Begin.\n')
	${Q}${RM} ${APP_NAME}.elf ${APP_OBJS} ${D_FILES}
	$(if ${Q},@printf '>>> CLEAN[${APP_NAME}.elf]: Done.\n')

endif

clean-${DRVNAME}.ko:
	$(if ${Q},@printf '>>> CLEAN[${DRVNAME}.ko]: Begin.\n')
	${Q}${MAKE} -C ${ROOT} M=`pwd` ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} clean # modname=${DRVNAME}
	${Q}for i in $(filter ../% /%, $(sort $(dir ${${DRVNAME}-objs}))); \
	do \
		rm -f $${i}*.o $${i}.*.o.cmd; \
	done
	$(if ${Q},@printf '>>> CLEAN[${DRVNAME}.ko]: Done.\n')

clean: $(if ${APP_NAME}, clean-${APP_NAME}.elf) clean-${DRVNAME}.ko

$(foreach i, ${ARCH_LIST}, clean-${i}): %:
	${Q}${MAKE} clean ARCH=${@:clean-%=%}

__VARS__ := ARCH_LIST HOST_ARCH ARCH $(foreach i, ${ARCH_LIST}, CROSS_COMPILE_FOR_${i}) CROSS_COMPILE \
    CC STRIP RM __STRICT__ NDEBUG USE_SRC_RELATIVE_PATH \
    HOST_KERNEL_DIR CROSS_KERNEL_DIR DRVNAME ${DRVNAME}-objs obj-m ccflags-y \
    APP_NAME APP_OBJS APP_DEBUG_FLAGS APP_DEFINES APP_INCLUDES OTHER_APP_CFLAGS APP_CFLAGS \
    PARALLEL_OPTION

ifeq (${Q},)
    $(info -)
    $(foreach i, ${__VARS__}, \
        $(eval \
            $(info \
                - ${i}: ${$i} \
            ) \
        ) \
    )
    $(info -)
    $(info You can override most of variables above to meet your need.)
    $(info -)
else
    ifeq (${MAKELEVEL}, 0)
        $(info Run with "V=1" or "VERBOSE=1" if you're interested in compilation details.)
    endif
endif

endif # ifeq (${KERNELRELEASE},)

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-04-07, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2023-04-08, Man Hung-Coeng <udc577@126.com>:
#   01. Fix the bug of ${DRVNAME}.ko not recompiling while it's sources are changed.
#
# >>> 2023-04-16, Man Hung-Coeng <udc577@126.com>:
#   01. Add ${PREREQUISITES} to "all" target to make it possible
#       to make some optional preparations before compilation.
#   02. Specify target "all", "debug" and "clean" as ".PHONY".
#   03. Add dependencies for auto-detection of header content update.
#
# >>> 2023-07-14, Man Hung-Coeng <udc577@126.com>:
#   01. Support multiple architectures.
#
# >>> 2023-07-19, Man Hung-Coeng <udc577@126.com>:
#   01. Make the application demo program optional.
#   02. Rename targets: clean-driver -> clean-${DRVNAME}.ko,
#       clean-app -> clean-${APP_NAME}.elf.
#
# >>> 2023-09-27, Man Hung-Coeng <udc577@126.com>:
#   01. Make some operations be conditionally executed depending on value of
#   	KERNELRELEASE, so as to accelerate driver compilation
#   	and meanwhile avoid some conflicts.
#
# >>> 2023-11-07, Man Hung-Coeng <udc577@126.com>:
#   01. Add macro USE_SRC_RELATIVE_PATH and __SRC__.
#   02. Support deletion of *.o and .*.o.cmd files outside current directory.
#

