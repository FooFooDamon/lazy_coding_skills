#
# Makefile wrapper for Linux kernel.
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
export KBUILD_BUILD_VERSION ?= ${__VER__}
KCFLAGS ?= -DUTS_NODENAME=\\\"`hostname`[${__VER__}]\\\" -DUTS_DOMAINNAME=\\\"${VCS}://ver.${__VER__}.nil/\\\"
MAKE_ARGS := ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} $(if ${__VER__},KCFLAGS="${KCFLAGS}")
CP ?= cp -R -P
DIFF ?= diff --color
TOUCH ?= touch
UNCOMPRESS ?= tar -zxvf
PKG_FILE ?= ./linux-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
PKG_URL ?= https://github.com/nxp-imx/linux-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
PKG_DOWNLOAD ?= wget -c '$(strip ${PKG_URL})' -O ${PKG_FILE}
SRC_PARENT_DIR ?= ./_tmp_
SRC_ROOT_DIR ?= $(shell \
    [ -f ${PKG_FILE} -o -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || ${PKG_DOWNLOAD} >&2; \
    [ -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (mkdir -p ${SRC_PARENT_DIR}; ${UNCOMPRESS} ${PKG_FILE} -C ${SRC_PARENT_DIR}) >&2; \
    ls -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) \
)
KERNEL_IMAGE ?= zImage
DTS_PATH ?= arch/arm/boot/dts/imx6ull-14x14-evk.dts
INSTALL_DIR ?= ${HOME}/tftpd
INSTALL_CMD ?= [ -d ${INSTALL_DIR} ] || mkdir -p ${INSTALL_DIR}; \
    find ${SRC_ROOT_DIR}/arch/${ARCH}/boot/ -name ${KERNEL_IMAGE} -exec ${CP} {} ${INSTALL_DIR}/ \; ; \
    $(if ${DTS_PATH},${CP} ${SRC_ROOT_DIR}/${DTS_PATH:%.dts=%.dtb} ${INSTALL_DIR}/)
UNINSTALL_CMD ?= rm -f ${INSTALL_DIR}/${KERNEL_IMAGE} $(if ${DTS_PATH},${INSTALL_DIR}/$(notdir ${DTS_PATH:%.dts=%.dtb}))
DEFCONFIG ?= arch/arm/configs/imx_v7_defconfig
EXTRA_TARGETS ?= dtbs drivers/net/can/usb/peak_usb/peak_usb.ko
CUSTOM_FILES ?= ${DEFCONFIG} \
    ${DTS_PATH}
__CUSTOMIZED_DEPENDENCIES := $(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i})

$(foreach i, DEFCONFIG EXTRA_TARGETS, $(eval ${i} := $(strip ${${i}})))

.PHONY: ${KERNEL_IMAGE} all menuconfig $(if ${DEFCONFIG},defconfig) download clean distclean install uninstall ${EXTRA_TARGETS}

${KERNEL_IMAGE} all: ${__CUSTOMIZED_DEPENDENCIES}
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS} -j $$(grep -c processor /proc/cpuinfo)

menuconfig: $(if ${DEFCONFIG},defconfig) ${__CUSTOMIZED_DEPENDENCIES}
	[ -z "${DEFCONFIG}" ] && conf_file=.config || conf_file=${DEFCONFIG}; \
	[ "$${conf_file}" == ".config" -a -f $${conf_file} ] && ${CP} $${conf_file} ${SRC_ROOT_DIR}/.config || : ; \
	${MAKE} menuconfig -C ${SRC_ROOT_DIR} ${MAKE_ARGS}; \
	if [ -f ${SRC_ROOT_DIR}/.config ]; then \
		set -x; \
		[ -f $${conf_file} ] && ${DIFF} ${SRC_ROOT_DIR}/.config $${conf_file} || ${CP} ${SRC_ROOT_DIR}/.config $${conf_file}; \
	fi

ifneq (${DEFCONFIG},)

defconfig: ${SRC_ROOT_DIR}/${DEFCONFIG}
	${MAKE} $(notdir ${DEFCONFIG}) -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

endif

download:
	[ -f ${PKG_FILE} ] || ${PKG_DOWNLOAD}

install:
	${INSTALL_CMD}

uninstall:
	${UNINSTALL_CMD}

${EXTRA_TARGETS}: ${__CUSTOMIZED_DEPENDENCIES}

clean distclean ${EXTRA_TARGETS}: %:
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

define custom_file_rule
${SRC_ROOT_DIR}/${1}: ${1}
	${DIFF} ${SRC_ROOT_DIR}/${1} ${1} && ${TOUCH} ${SRC_ROOT_DIR}/${1} || ${CP} ${1} ${SRC_ROOT_DIR}/${1}
endef

$(foreach i, ${CUSTOM_FILES}, $(eval $(call custom_file_rule,${i})))

help:
	@echo "Core directives:"
	@echo "  1. make download   - Download the source package manually; Usually unnecessary"
	@echo "  2. make menuconfig - Interactive configuration (automatically saved if changed)"
	@echo "  3. make            - Build in a default way (with or without \"all\")"
	@echo "  4. make ${KERNEL_IMAGE}     - Build the kernel image"
	@echo "  5. make clean      - Clean most generated files and directories"
	@echo "  6. make distclean  - Clean all generated files and directories (including .config)"
	@echo "  7. make install    - Copy ${KERNEL_IMAGE} $(if ${DTS_PATH},and ${DTS_PATH:%.dts=%.dtb}) to the directory specified by INSTALL_DIR"
	@echo "  8. make uninstall  - Delete ${KERNEL_IMAGE} $(if ${DTS_PATH},and ${DTS_PATH:%.dts=%.dtb}) in the directory specified by INSTALL_DIR"
	@echo ""
	@printf "Extra directive(s): "
ifeq (${EXTRA_TARGETS},)
	@echo "None"
else
	@for i in ${EXTRA_TARGETS}; \
	do \
		printf "\n  * make $${i}"; \
	done
	@printf "\n  --"
	@printf "\n  Run \"make help\" in directory[${SRC_ROOT_DIR}]"
	@printf "\n  to see detailed descriptions."
	@printf "\n"
endif

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2024-02-07, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2024-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Change the non-error output redirection of Shell commands of
#       SRC_ROOT_DIR definition from /dev/null to stderr.
#   02. Beautify the display of extra directive(s) of "make help".
#   03. Make target EXTRA_TARGETS depend on CUSTOM_FILES.
#
# >>> 2024-02-18, Man Hung-Coeng <udc577@126.com>:
#   01. Make target "menuconfig" depend on CUSTOM_FILES.
#

