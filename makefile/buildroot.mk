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

__VER__ ?= 123456789abc
PKG_VERSION ?= 2023.02
PKG_FILE ?= ./buildroot-${PKG_VERSION}.tar.gz
PKG_URL ?= https://buildroot.org/downloads/buildroot-${PKG_VERSION}.tar.gz
PKG_DOWNLOAD ?= wget -c '$(strip ${PKG_URL})' -O ${PKG_FILE}
MAKE_ARGS ?= $(if ${__VER__},BR2_VERSION=${PKG_VERSION}-${__VER__})
CP ?= cp -R -P
DIFF ?= diff --color
TOUCH ?= touch
UNCOMPRESS ?= tar -zxvf
SRC_PARENT_DIR ?= ${HOME}/src
SRC_ROOT_DIR ?= $(shell \
    [ -f ${PKG_FILE} -o -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (${PKG_DOWNLOAD}) >&2; \
    [ -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (mkdir -p ${SRC_PARENT_DIR}; ${UNCOMPRESS} ${PKG_FILE} -C ${SRC_PARENT_DIR}) >&2; \
    ls -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) \
)
INSTALL_DIR ?= ${HOME}/tftpd
INSTALL_CMD ?= [ -d ${INSTALL_DIR} ] || mkdir -p ${INSTALL_DIR}; ${CP} ${SRC_ROOT_DIR}/output/images/rootfs.* ${INSTALL_DIR}/
UNINSTALL_CMD ?= rm -f ${INSTALL_DIR}/rootfs.*
EXT_TARGETS += source
BUSYBOX_CONFIG ?= package/busybox/busybox.config
OVERLAY_DIR ?= overlay
CUSTOM_FILES += ${BUSYBOX_CONFIG}

$(foreach i, EXT_TARGETS, $(eval ${i} := $(strip ${${i}})))
$(foreach i, EXT_TARGETS CUSTOM_FILES, $(eval ${i} := $(sort ${${i}}))) # To filter out repeated items

.PHONY: all menuconfig busybox-menuconfig overlay_dir \
    download clean distclean install uninstall ${EXT_TARGETS} help showvars

all: overlay_dir $(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i})
	${MAKE} -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

menuconfig: overlay_dir
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

overlay_dir:
	[ -d ${OVERLAY_DIR} ] || mkdir ${OVERLAY_DIR}
	[ -d ${SRC_ROOT_DIR}/${OVERLAY_DIR} ] || ln -snf $$(realpath ${OVERLAY_DIR}) ${SRC_ROOT_DIR}/${OVERLAY_DIR}

download:
	[ -f ${PKG_FILE} ] || (${PKG_DOWNLOAD})

install:
	${INSTALL_CMD}

uninstall:
	${UNINSTALL_CMD}

clean distclean ${EXT_TARGETS}: %:
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS}
	@if [ $@ = clean -o $@ = distclean ]; then \
		printf '\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'; \
		printf '\n!!!!!!!!!! ATTENTION: !!!!!!!!!!'; \
		printf '\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'; \
		printf '\n'; \
		printf '\nNext time before you execute "${MAKE}",'; \
		printf '\nexecute $(if $(filter distclean, $@),"${MAKE} menuconfig" and )"${MAKE} busybox-menuconfig" first!'; \
		printf '\n\n'; \
	fi

define custom_file_rule
${SRC_ROOT_DIR}/${1}: ${1}
	${DIFF} ${SRC_ROOT_DIR}/${1} ${1} && ${TOUCH} ${SRC_ROOT_DIR}/${1} || ${CP} ${1} ${SRC_ROOT_DIR}/${1}
endef

$(foreach i, ${CUSTOM_FILES}, $(eval $(call custom_file_rule,${i})))

help:
	@echo "Core directives:"
	@echo "  1. ${MAKE} download   - Download the source package manually; Usually unnecessary"
	@echo "  2. ${MAKE} showvars   - Display customized variables and their values"
	@echo "  3. ${MAKE} menuconfig - Interactive configuration (automatically saved if changed)"
	@echo "  4. ${MAKE} busybox-menuconfig"
	@echo "                     - Interactive configuration for BusyBox (automatically saved if changed)"
	@echo "  5. ${MAKE}            - Build in a default way"
	@echo "  6. ${MAKE} clean      - Clean most generated files and directories"
	@echo "  7. ${MAKE} distclean  - Clean all generated files and directories (including .config and downloaded packages)"
	@echo "  8. ${MAKE} install    - Copy the generated rootfs.* files to the directory specified by INSTALL_DIR"
	@echo "  9. ${MAKE} uninstall  - Delete rootfs.* files in the directory specified by INSTALL_DIR"
	@echo ""
	@printf "Extended directive(s): "
ifeq (${EXT_TARGETS},)
	@echo "None"
else
	@for i in ${EXT_TARGETS}; \
	do \
		printf "\n  * ${MAKE} $${i}"; \
	done
	@printf "\n  --"
	@printf "\n  Run \"${MAKE} help -C ${PWD}/${SRC_ROOT_DIR}\""
	@printf "\n  to see detailed descriptions."
	@printf "\n"
endif
	$(if $(strip ${USER_HELP_PRINTS}),@printf "\nUser help info:\n"; (${USER_HELP_PRINTS}))

__VARS__ := CP DIFF TOUCH UNCOMPRESS \
    __VER__ PKG_VERSION MAKE_ARGS  \
    PKG_FILE PKG_URL PKG_DOWNLOAD SRC_PARENT_DIR SRC_ROOT_DIR \
    INSTALL_DIR INSTALL_CMD UNINSTALL_CMD \
    BUSYBOX_CONFIG OVERLAY_DIR EXT_TARGETS CUSTOM_FILES USER_HELP_PRINTS

showvars:
	$(info )
	$(foreach i, ${__VARS__}, $(info ${i} = ${$i}) $(info ))
	@:

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2024-02-04, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2024-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Change the non-error output redirection of Shell commands of
#       SRC_ROOT_DIR definition from /dev/null to stderr.
#   02. Beautify the display of extra directive(s) of "make help".
#
# >>> 2024-03-02, Man Hung-Coeng <udc577@126.com>:
#   01. Rename CUSTOM_DIR to OVERLAY_DIR, custom_dir to overlay_dir.
#   02. Change the assignment operator of EXTRA_TARGETS (new name: EXT_TARGETS)
#       and CUSTOM_FILES to addition assignment operator (+=).
#   03. Add a new target "showvars".
#   04. Enhance "make help" by allowing to define extra printing commands.
#

