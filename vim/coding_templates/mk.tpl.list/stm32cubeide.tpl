#
# Makefile wrapper for STM32CubeIDE project.
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

# export CPU_SERIES ?= stm32f1x
LAZY_CODING_MAKEFILES := stm32_cube_ide.mk # ${CPU_SERIES}_private.mk

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

#
# FIXME: Uncomment and modify lines below according to your needs.
#
# export MODE ?= Release
# export IDE_PARENT_DIR ?= /opt/st
# export DEBUG_DEVICE ?= stlink
# export FLASH_ADDR_START := 0x08000000
# export RAM_ADDR_START := 0x20000000

include ${LAZY_CODING_MAKEFILES}

export DEPENDENCY_DIRS ?= ../3rdparty

prepare:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] && ${MAKE} -C $${i} || true; \
	done

# FIXME: Add more rules if needed, and delete this comment line then.

endif
