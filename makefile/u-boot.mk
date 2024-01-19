#
# Makefile wrapper for U-Boot.
#
# Copyright (c) 2024 Man Hung-Coeng <udc577@126.com>
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

ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
MAKE_ARGS := ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} $(if ${__VER__},EXTRAVERSION=-${__VER__})
CP ?= cp --no-dereference --preserve=links,timestamps --update
DIFF ?= diff --color
TOUCH ?= touch
UNCOMPRESS ?= tar -zxvf
SRC_PKG_FILE ?= ./uboot-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
SRC_PKG_URL ?= https://github.com/nxp-imx/uboot-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
SRC_PKG_DOWNLOAD ?= wget -c '$(strip ${SRC_PKG_URL})' -O ${SRC_PKG_FILE}
SRC_PARENT_DIR ?= ./_tmp_
SRC_ROOT_DIR ?= $(shell \
    [ -f ${SRC_PKG_FILE} -o -d ${SRC_PARENT_DIR}/u*boot-* ] || ${SRC_PKG_DOWNLOAD} > /dev/null; \
    [ -d ${SRC_PARENT_DIR}/u*boot-* ] || (mkdir -p ${SRC_PARENT_DIR}; ${UNCOMPRESS} ${SRC_PKG_FILE} -C ${SRC_PARENT_DIR}) > /dev/null; \
    ls -d ${SRC_PARENT_DIR}/u*boot-* \
)
DEFCONFIG ?= configs/mx6ull_14x14_evk_nand_defconfig
EXTRA_TARGETS ?= dtbs
CUSTOM_FILES ?= ${DEFCONFIG} \
    include/configs/mx6ullevk.h \
    board/freescale/mx6ullevk/mx6ullevk.c \
    drivers/net/phy/ti.c

$(foreach i, DEFCONFIG EXTRA_TARGETS, $(eval ${i} := $(strip ${${i}})))

.PHONY: all menuconfig $(if ${DEFCONFIG},defconfig) download clean distclean ${EXTRA_TARGETS}

all: $(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i})
	${MAKE} -C ${SRC_ROOT_DIR} ${MAKE_ARGS} -j $$(grep -c processor /proc/cpuinfo)

menuconfig: $(if ${DEFCONFIG},defconfig)
	[ -z "${DEFCONFIG}" ] && conf_file=.config || conf_file=${DEFCONFIG}; \
	[ "$${conf_file}" == ".config" -a -f $${conf_file} ] && ${CP} $${conf_file} ${SRC_ROOT_DIR}/.config || : ; \
	${MAKE} menuconfig -C ${SRC_ROOT_DIR} ${MAKE_ARGS} EXTRAVERSION=""; \
	if [ -f ${SRC_ROOT_DIR}/.config ]; then \
		set -x; \
		[ -f $${conf_file} ] && ${DIFF} ${SRC_ROOT_DIR}/.config $${conf_file} || ${CP} ${SRC_ROOT_DIR}/.config $${conf_file}; \
	fi

ifneq (${DEFCONFIG},)

defconfig: ${SRC_ROOT_DIR}/${DEFCONFIG}
	${MAKE} $(notdir ${DEFCONFIG}) -C ${SRC_ROOT_DIR} ${MAKE_ARGS} EXTRAVERSION=""

endif

download:
	[ -f ${SRC_PKG_FILE} ] || ${SRC_PKG_DOWNLOAD}

clean distclean ${EXTRA_TARGETS}: %:
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

define custom_file_rule
${SRC_ROOT_DIR}/${1}: ${1}
	${DIFF} ${SRC_ROOT_DIR}/${1} ${1} && ${TOUCH} ${SRC_ROOT_DIR}/${1} || ${CP} ${1} ${SRC_ROOT_DIR}/${1}
endef

$(foreach i, ${CUSTOM_FILES}, $(eval $(call custom_file_rule,${i})))

# Failure case (1): target 'xxx' doesn't match the target pattern
#${CUSTOM_FILES}: ${SRC_ROOT_DIR}/%: %
# Failure case (2): substitution failure which leads to circular dependency
#$(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i}): %: $(subst ${SRC_ROOT_DIR}/,,%)
#	${DIFF} $@ $< && ${TOUCH} $@ || ${CP} $< $@ # Shared by (1) and (2)

# Failure case (3): never triggered
#$(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i}): %:
# Failure case (4): almost a success except for the thundering herd problem
#$(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i}): %: ${CUSTOM_FILES}
#	${DIFF} $@ ${@:${SRC_ROOT_DIR}/%=%} && ${TOUCH} $@ || ${CP} ${@:${SRC_ROOT_DIR}/%=%} $@ # Shared by (3) and (4)

NULL :=
SPACE := ${NULL} ${NULL}
# It make no difference whether \t and \n are defined or not.
\t := ${NULL}	${NULL}
define \n


endef

# Failure case (5), (6), (7): unable to parse Tab and Newline correctly
#$(foreach i, ${CUSTOM_FILES}, $(eval ${SRC_ROOT_DIR}/${i}: ${i}\n\t${DIFF} $@ $< && ${TOUCH} $@ || ${CP} $< $@))
#$(foreach i, ${CUSTOM_FILES}, $(eval $(shell printf "${SRC_ROOT_DIR}/${i}: ${i}\n\t${DIFF} $@ $< && ${TOUCH} $@ || ${CP} $< $@\n")))
#$(eval $(foreach i, ${CUSTOM_FILES}, $(shell printf "${SRC_ROOT_DIR}/${i}: ${i}\n\t${DIFF} $@ $< && ${TOUCH} $@ || ${CP} $< $@\n")))

help:
	@echo "Core directives:"
	@echo "  1. make download   - Download the source package manually; Usually unnecessary"
	@echo "  2. make distclean  - Clean all generated files and directories including .config"
	@echo "  3. make defconfig  - Rough configuration; Invoked by \"make menuconfig\" automatically"
	@echo "  4. make menuconfig - Detailed configuration (automatically saved if changed)"
	@echo "  5. make clean      - Clean most generated files and directories"
	@echo "  6. make            - Build U-Boot in a default way"
	@echo ""
	@printf "Extra directive(s): "
ifeq (${EXTRA_TARGETS},)
	@echo "None"
else
	@echo "make $(if $(findstring ${SPACE},${EXTRA_TARGETS}),{$(subst ${SPACE},|,${EXTRA_TARGETS})},${EXTRA_TARGETS})"
	@echo "  Run \"make help\" in directory[${SRC_ROOT_DIR}]"
	@echo "  to see detailed descriptions."
endif

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2024-01-18, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2024-01-19, Man Hung-Coeng <udc577@126.com>:
#   01. Fix the bug of infinite recursion (due to the invocation of
#       "make download" within the SRC_ROOT_DIR definition block) in absence of
#       ${SRC_PKG_FILE} and ${SRC_PARENT_DIR}/u*boot-*.
#   02. Strip heading and trailing blanks of several variables
#       for the sake of robustness and conciseness.
#   03. Deduce the resulting file of "make menuconfig" intelligently.
#

