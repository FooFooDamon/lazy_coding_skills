#!/usr/bin/make -f

#
# TODO: Brief description of this makefile.
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




# Make sure that "all" is the first and the only target in this file.
all: init




LCS_ALIAS ?= lazy_coding
# FIXME: You probably want to modify this directory.
# For a formal project, $(abspath .) or somewhere else might be better.
THIRD_PARTY_DIR ?= ${HOME}/src
# T is short for "type",
# which Should be one of: app driver stm32cubeide stm32raw
T := app
EVAL_VERSION_ONCE := $(if $(filter driver, ${T}), Y, N)

-include ${THIRD_PARTY_DIR}/${LCS_ALIAS}/makefile/__ver__.mk




#=======================
# Application
#=======================
ifeq (${T}, app)

#
# Make sure that:
# 1) C_SRCS contains all C source files and
# 2) CXX_SRCS contains all C++ source files and
# 3) Source files in other directories must be included as well.
#
C_SRCS := $(shell find ./ -name "*.c")
CXX_SRCS := $(shell find ./ -name "*.cpp" -o -name "*.cc" -o -name "*.cxx")

# GOAL is a mandatory target.
# XXX is whatever name you like.
# If you expect a static or shared library,
# then GOAL should be named libXXX.a or libXXX.so.
GOAL := XXX.elf

# Just defining dependencies is enough.
# You probably want to add extra source files, or filter some unneeded ones.
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

-include ${THIRD_PARTY_DIR}/${LCS_ALIAS}/makefile/c_and_cpp.mk


#=======================
# Linux Driver
#=======================
else ifeq (${T}, driver)

#
# FIXME: Uncomment and modify lines below according to your needs.
#
# ARCH_LIST := arm host x86
# ARCH := arm
# export HOST_KERNEL_DIR := /lib/modules/`uname -r`/build
# export CROSS_KERNEL_DIR := ${HOME}/src/linux
# export DRVNAME := xxx
# export ${DRVNAME}-objs := xxx_main.o xxx_utils.o
# export APP_NAME := xxx_app
# export APP_OBJS := xxx_app_main.o xxx_app_utils.o
# Other settings if needed: APP_DEFINES, APP_INCLUDES, OTHER_APP_CFLAGS, etc.

-include ${THIRD_PARTY_DIR}/${LCS_ALIAS}/makefile/linux_driver.mk


#=======================
# STM32CubeIDE
#=======================
else ifeq (${T}, stm32cubeide)

#
# FIXME: Uncomment and modify lines below according to your needs.
#
# export CPU_SERIES := stm32f1x
# export FLASH_ADDR_START := 0x08000000
# export RAM_ADDR_START := 0x20000000

-include ${THIRD_PARTY_DIR}/${LCS_ALIAS}/makefile/stm32_cube_ide.mk
ifeq ($(filter $(abspath .) $(realpath .) ., ${THIRD_PARTY_DIR}),)
    -include ${THIRD_PARTY_DIR}/${LCS_ALIAS}/makefile/${CPU_SERIES}_private.mk
endif


#=======================
# STM32 Raw
#=======================
else ifneq ($(filter ${T}, stm32raw),)

$(error Makefile type not supported yet: ${T})


else

$(error Unrecognized Makefile type: ${T})

endif




VCS_LIST := git svn
VCS ?= $(word 1, ${VCS_LIST})
LCS_URL ?= https://github.com/FooFooDamon/lazy_coding_skills
# Format of each project item: <alias>@@<method>@@<vcs>@@<default-branch>@@<url>
# NOTES:
# 1) If the method field of an item is by-tag or by-hash,
#   then its tag or hash code needs to be set into file manually
#   after "make seeds" and before "make init".
# 2) The "partial" method only works in HTTP(S) way so far.
# 3) SVN projects are not supported yet.
THIRD_PARTY_PROJECTS ?= ${LCS_ALIAS}@@partial@@git@@main@@${LCS_URL} \
    #nvidia-docker-v2.12.0@@by-tag@@git@@main@@https://gitlab.com/nvidia/container-toolkit/nvidia-docker \
    #nvidia-docker-80902fe3afab@@by-hash@@git@@main@@git@gitlab.com:nvidia/container-toolkit/nvidia-docker.git \
    #rt-thread@@by-tag@@git@@master@@https://gitee.com/rtthread/rt-thread.git \
	# FIXME: Add more items ahead of this line if needed. \
    # Beware that each line should begin with 4 spaces and end with a backslash.
CHKOUT ?= ${LCS_ALIAS}

seeds:
	$(if ${Q},@printf '>>> SEEDS: Begin.\n')
	${Q}mkdir -p ${THIRD_PARTY_DIR}
	${Q}for i in ${VCS_LIST}; \
	do \
		[ ! -s ${THIRD_PARTY_DIR}/checkout.$${i}.mk ] || continue; \
		$(if ${Q},printf "WGET\t${THIRD_PARTY_DIR}/checkout.$${i}.mk\n";) \
		wget $(if ${Q},-q) -c -O ${THIRD_PARTY_DIR}/checkout.$${i}.mk "${LCS_URL}/raw/main/makefile/checkout.$${i}.mk"; \
	done
	${Q}for i in ${THIRD_PARTY_PROJECTS}; \
	do \
		export CHKOUT_ALIAS=$$(echo "$${i}" | awk -F '@@' '{ print $$1 }'); \
		export CHKOUT_METHOD=$$(echo "$${i}" | awk -F '@@' '{ print $$2 }'); \
		export VCS_CMD=$$(echo "$${i}" | awk -F '@@' '{ print $$3 }'); \
		export CHKOUT_STEM=$$(echo "$${i}" | awk -F '@@' '{ print $$4 }'); \
		export CHKOUT_URL=$$(echo "$${i}" | awk -F '@@' '{ print $$5 }'); \
		export MKFILE=${THIRD_PARTY_DIR}/$${CHKOUT_ALIAS}.$${VCS_CMD}.chkout.mk; \
		[ ! -e $${MKFILE} ] || continue; \
		echo "export CHKOUT_PARENT_DIR := ${THIRD_PARTY_DIR}" > $${MKFILE}; \
		echo "export CHKOUT_ALIAS := $${CHKOUT_ALIAS}" >> $${MKFILE}; \
		echo "export CHKOUT_TAG :=" >> $${MKFILE}; \
		echo "export CHKOUT_HASH :=" >> $${MKFILE}; \
		echo "export CHKOUT_STEM := $${CHKOUT_STEM}" >> $${MKFILE}; \
		echo "export CHKOUT_URL := $${CHKOUT_URL}" >> $${MKFILE}; \
		echo "export CHKOUT_TAIL_PARAMS :=" >> $${MKFILE}; \
		echo "export CHKOUT_METHOD := $${CHKOUT_METHOD}" >> $${MKFILE}; \
		if [ "$${CHKOUT_ALIAS}" = "${LCS_ALIAS}" ]; then \
			echo "export CHKOUT_PARTIAL_ITEMS := main/makefile/__ver__.mk \\" >> $${MKFILE}; \
			echo "    adfce11edd4fa36f324f0e1ac6d4396341cc5266/makefile/c_and_cpp.mk \\" >> $${MKFILE}; \
			echo "    main/makefile/linux_driver.mk \\" >> $${MKFILE}; \
			echo "    main/makefile/stm32_cube_ide.mk \\" >> $${MKFILE}; \
		else \
			echo "export CHKOUT_PARTIAL_ITEMS := \\" >> $${MKFILE}; \
		fi; \
		echo "    # Add more items ahead of this line if needed. \\" >> $${MKFILE}; \
		echo "    # Beware that each line should begin with 4 spaces and end with a backslash." >> $${MKFILE}; \
		echo "" >> $${MKFILE}; \
		echo "[$${MKFILE}] has been created. Edit it properly before use."; \
	done
	$(if ${Q},@printf '>>> SEEDS: Done.\n')

init:
	$(if ${Q},@printf '>>> INIT: Begin.\n')
	${Q}for i in ${VCS_LIST}; \
	do \
		[ ! -s ${THIRD_PARTY_DIR}/checkout.$${i}.mk ] || continue; \
		echo "*** [${THIRD_PARTY_DIR}/checkout.$${i}.mk] is empty, or does not exist!" >&2; \
		echo '*** Run "make seeds" to check it out first!' >&2; \
		exit 1; \
	done
	${Q}for i in ${THIRD_PARTY_PROJECTS}; \
	do \
		export CHKOUT_ALIAS=$$(echo "$${i}" | awk -F '@@' '{ print $$1 }'); \
		export VCS_CMD=$$(echo "$${i}" | awk -F '@@' '{ print $$3 }'); \
		export MKFILE=${THIRD_PARTY_DIR}/$${CHKOUT_ALIAS}.$${VCS_CMD}.chkout.mk; \
		if [ ! -e $${MKFILE} ]; then \
			echo "*** [$${MKFILE}] does not exist!" >&2; \
			echo '*** Run "make seeds" to create it first!' >&2; \
			exit 1; \
		fi; \
		ask_and_quit() { echo "*** Have you modified [$${MKFILE}] correctly ?!" >&2; exit 1; }; \
		$(if ${Q},printf ">>> CHKOUT: Begin checking out [$${CHKOUT_ALIAS}].\n";) \
		${MAKE} $(if ${Q},-s) checkout VCS=$${VCS_CMD} CHKOUT=$${CHKOUT_ALIAS} || ask_and_quit; \
		$(if ${Q},printf ">>> CHKOUT: Done checking out [$${CHKOUT_ALIAS}].\n";) \
	done
	$(if ${Q},@printf '>>> INIT: Done.\n')

-include ${THIRD_PARTY_DIR}/${CHKOUT}.${VCS}.chkout.mk
-include ${THIRD_PARTY_DIR}/checkout.${VCS}.mk




#
# If you want to re-define commands of some targets, write them here:
#

# FIXME: More rules if needed ...

#
# ================
#   CHANGE LOG
# ================
#
# >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
#   01. Create.
#
