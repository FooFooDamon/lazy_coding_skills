#
# Makefile wrapper for Buildroot.
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

PKG_VERSION ?= 2023.02
PKG_FILE ?= ./buildroot-${PKG_VERSION}.tar.gz
PKG_URL ?= https://buildroot.org/downloads/buildroot-${PKG_VERSION}.tar.gz
PKG_DOWNLOAD ?= wget -c '$(strip ${PKG_URL})' -O ${PKG_FILE}
MAKE_ARGS := $(if ${__VER__},BR2_VERSION=${PKG_VERSION}-${__VER__})
CP ?= cp -R -P
DIFF ?= diff --color
TOUCH ?= touch
UNCOMPRESS ?= tar -zxvf
SRC_PARENT_DIR ?= ${HOME}/src
SRC_ROOT_DIR ?= $(shell \
    [ -f ${PKG_FILE} -o -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || ${PKG_DOWNLOAD} > /dev/null; \
    [ -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (mkdir -p ${SRC_PARENT_DIR}; ${UNCOMPRESS} ${PKG_FILE} -C ${SRC_PARENT_DIR}) > /dev/null; \
    ls -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) \
)
INSTALL_DIR ?= ${PWD}
INSTALL_CMD ?= ${CP} ${SRC_ROOT_DIR}/output/images/rootfs.* ${INSTALL_DIR}/
UNINSTALL_CMD ?= rm -f ${INSTALL_DIR}/rootfs.*
EXTRA_TARGETS ?= source
BUSYBOX_CONFIG := package/busybox/busybox.config
CUSTOM_DIR ?= custom
CUSTOM_FILES ?= ${BUSYBOX_CONFIG} \
    package/busybox/0005-libbb-printable_string2.patch \
    package/busybox/0006-libbb-unicode_conv_to_printable2.patch

$(foreach i, EXTRA_TARGETS, $(eval ${i} := $(strip ${${i}})))

.PHONY: all menuconfig busybox-menuconfig custom_dir download clean distclean install uninstall ${EXTRA_TARGETS}

all: custom_dir $(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i})
	${MAKE} -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

menuconfig: custom_dir
	[ -f .config ] && ${CP} .config ${SRC_ROOT_DIR}/ || :
	${MAKE} menuconfig -C ${SRC_ROOT_DIR} ${MAKE_ARGS}
	if [ -f ${SRC_ROOT_DIR}/.config ]; then \
		set -x; \
		[ -f .config ] && ${DIFF} ${SRC_ROOT_DIR}/.config .config || ${CP} ${SRC_ROOT_DIR}/.config ./; \
	fi

busybox-menuconfig: ${SRC_ROOT_DIR}/${BUSYBOX_CONFIG}
	${MAKE} busybox-menuconfig -C ${SRC_ROOT_DIR}
	${MAKE} busybox-update-config -C ${SRC_ROOT_DIR}
	[ -d $(dir ${BUSYBOX_CONFIG}) ] || mkdir -p $(dir ${BUSYBOX_CONFIG})
	set -x; \
	[ -f ${BUSYBOX_CONFIG} ] && ${DIFF} $< ${BUSYBOX_CONFIG} || ${CP} $< ${BUSYBOX_CONFIG}

custom_dir:
	[ -d ${CUSTOM_DIR} ] || mkdir ${CUSTOM_DIR}
	[ -d ${SRC_ROOT_DIR}/${CUSTOM_DIR} ] || ln -snf $$(realpath ${CUSTOM_DIR}) ${SRC_ROOT_DIR}/${CUSTOM_DIR}

download:
	[ -f ${PKG_FILE} ] || ${PKG_DOWNLOAD}

install:
	${INSTALL_CMD}

uninstall:
	${UNINSTALL_CMD}

clean distclean ${EXTRA_TARGETS}: %:
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS}
	@if [ $@ = clean -o $@ = distclean ]; then \
		printf '\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'; \
		printf '\n!!!!!!!!!! ATTENTION: !!!!!!!!!!'; \
		printf '\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'; \
		printf '\n'; \
		printf '\nNext time before you execute "make",'; \
		printf '\nexecute $(if $(filter distclean, $@),"make menuconfig" and )"make busybox-menuconfig" first!'; \
		printf '\n\n'; \
	fi

define custom_file_rule
${SRC_ROOT_DIR}/${1}: ${1}
	${DIFF} ${SRC_ROOT_DIR}/${1} ${1} && ${TOUCH} ${SRC_ROOT_DIR}/${1} || ${CP} ${1} ${SRC_ROOT_DIR}/${1}
endef

$(foreach i, ${CUSTOM_FILES}, $(eval $(call custom_file_rule,${i})))

NULL :=
SPACE := ${NULL} ${NULL}

help:
	@echo "Core directives:"
	@echo "  1. make download   - Download the source package manually; Usually unnecessary"
	@echo "  2. make menuconfig - Interactive configuration (automatically saved if changed)"
	@echo "  3. make busybox-menuconfig"
	@echo "                     - Interactive configuration for BusyBox (automatically saved if changed)"
	@echo "  4. make            - Build in a default way"
	@echo "  5. make clean      - Clean most generated files and directories"
	@echo "  6. make distclean  - Clean all generated files and directories (including .config and downloaded packages)"
	@echo "  7. make install    - Copy the generated rootfs.* files to the directory specified by INSTALL_DIR"
	@echo "  8. make uninstall  - Delete rootfs.* files in the directory specified by INSTALL_DIR"
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
# >>> 2024-02-04, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

