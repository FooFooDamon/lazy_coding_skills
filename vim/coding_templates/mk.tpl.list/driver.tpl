# SPDX-License-Identifier: GPL-2.0

#
# Makefile wrapper for Linux kernel porting project.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
#

.PHONY: all prepare

# NOTE: MUST use absolute paths!!! Same with others below.
LAZY_CODING_MAKEFILES := ${PWD}/__ver__.mk ${PWD}/linux_driver.mk

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

EVAL_VERSION_ONCE := Y
export NO_PRIV_STUFF := $(strip $(filter-out n N no NO No 0, ${NO_PRIV_STUFF}))

#
# FIXME: Uncomment and modify lines below according to your needs.
#
# ARCH := aarch64
# export HOST_KERNEL_DIR := /lib/modules/`uname -r`/build
# export CROSS_KERNEL_DIR := ${HOME}/src/linux
# export DRVNAME ?= xxx
# export ${DRVNAME}-objs ?= xxx_main.o xxx_utils.o
# export USE_SRC_RELATIVE_PATH ?= 1
# export APP_NAME ?= xxx_app
# export APP_OBJS ?= xxx_app_main.o xxx_app_utils.o
# Other settings if needed: APP_DEFINES, APP_INCLUDES, OTHER_APP_CFLAGS, ccflags-y, etc.

include ${LAZY_CODING_MAKEFILES}

export DEPENDENCY_DIRS ?= ${PWD}/../3rdparty

prepare:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] && ${MAKE} -C $${i} || true; \
	done

# FIXME: Add more rules if needed, and delete this comment line then.

endif
