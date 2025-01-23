# SPDX-License-Identifier: GPL-2.0

#
# Makefile wrapper for out-of-tree Linux driver.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
#

.PHONY: all prepare dependencies

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

# NOTE: Some paths must be absolute paths while some mustn't, and the rest don't care.
export LAZY_CODING_MAKEFILES ?= $(abspath __ver__.mk linux_driver.mk)

override undefine PREREQUISITE_FILES
PREREQUISITE_FILES := ${LAZY_CODING_URL}/raw/217ebcbe23a78dacf7491685ef2e121a0721eb62/c_and_cpp/native/klogging.h \
    ${LAZY_CODING_URL}/raw/5a4cf8c6554177a1ece6ea81d097a60430df84c2/c_and_cpp/native/evol_kernel.h \
    ${LAZY_CODING_URL}/raw/1fb9d6a00f89980ec4b9b72d056d58ca3bc752a6/c_and_cpp/native/chardev_group.c \
    ${LAZY_CODING_URL}/raw/73a04dd9a2a6f44bc4100951eeebcdbb91f4437c/c_and_cpp/native/chardev_group.h \

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES} $(notdir ${PREREQUISITE_FILES}), -a -s ${i}) ] && echo 1 || echo 0),0)

all prepare: dependencies
	@for i in ${LAZY_CODING_MAKEFILES}; \
	do \
		mkdir -p $$(dirname $${i}); \
		[ -s $${i} ] || wget -c -O $${i} "${LAZY_CODING_URL}/raw/main/makefile/$$(basename $${i})"; \
	done
	@for i in ${PREREQUISITE_FILES}; \
		[ -s $$(basename $${i}) ] || wget -c -O $$(basename $${i}) "$${i}"; \
	do \
	done
	@echo "~ ~ ~ Minimum preparation finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

all: dependencies

export EVAL_VERSION_ONCE ?= Y
export NO_PRIV_STUFF := $(strip $(filter-out n N no NO No 0, ${NO_PRIV_STUFF}))

#
# FIXME: Uncomment and modify lines below according to your needs.
#
# export ARCH := aarch64
# export HOST_KERNEL_DIR := /lib/modules/`uname -r`/build
# export CROSS_KERNEL_DIR := ${HOME}/src/linux
# export DRVNAME ?= xxx
# export ${DRVNAME}-objs ?= xxx_main.o xxx_utils.o
# export USE_SRC_RELATIVE_PATH ?= 1
# export APP_NAME ?= xxx_app
# export APP_OBJS ?= xxx_app_main.o xxx_app_utils.o
# Other settings if needed: APP_DEFINES, APP_INCLUDES, OTHER_APP_CFLAGS, ccflags-y, etc.

include ${LAZY_CODING_MAKEFILES}

# FIXME: Add more rules if needed, and delete this comment line then.

endif

export DEPENDENCY_DIRS ?= $(abspath ../3rdparty)

dependencies:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] && ${MAKE} $(filter all prepare, ${MAKECMDGOALS}) -C $${i} || true; \
	done
