# SPDX-License-Identifier: Apache-2.0

#
# Makefile wrapper for STM32CubeIDE project.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#

.PHONY: all prepare distclean dependencies

# FIXME: Choose the right CPU series.
CPU_SERIES ?= stm32f1x
# Be aware that ${CPU_SERIES}_extra.mk should be downloaded and be preferred over stm32_extra.mk if it exists.
LAZY_CODING_MAKEFILES ?= $(abspath stm32_cube_ide.mk stm32_extra.mk) # $(abspath ${CPU_SERIES}_extra.mk)

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES}, -a -s ${i}) ] && echo 1 || echo 0),0)

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

all prepare: dependencies
	@for i in ${LAZY_CODING_MAKEFILES}; \
	do \
		mkdir -p $$(dirname $${i}); \
		[ -s $${i} ] || wget -c -O $${i} "${LAZY_CODING_URL}/raw/main/makefiles/$$(basename $${i})"; \
	done
	@echo "~ ~ ~ Minimum preparation finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

all: dependencies

#
# FIXME: Uncomment and modify lines below according to your needs.
#
# MODE ?= Release
# IDE_PARENT_DIR ?= /opt/st
# DEBUG_DEVICE ?= stlink
# FLASH_ADDR_START := 0x08000000
# RAM_ADDR_START := 0x20000000

include ${LAZY_CODING_MAKEFILES}

# FIXME: Add more rules if needed, and delete this comment line then.

endif

DEPENDENCY_DIRS := $(abspath ./3rdparty)

dependencies:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] || continue; \
		${MAKE} -C $${i} $(filter all prepare distclean, ${MAKECMDGOALS}); \
	done
