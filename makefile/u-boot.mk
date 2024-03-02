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
__VER__ ?= 123456789abc
MAKE_ARGS ?= ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} $(if ${__VER__},EXTRAVERSION=-${__VER__})
NTHREADS ?= $(shell nproc)
CP ?= cp -R -P
DIFF ?= diff --color
TOUCH ?= touch
UNCOMPRESS ?= tar -zxvf
PKG_FILE ?= ./uboot-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
# Rule of URL: https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives
PKG_URL ?= https://github.com/nxp-imx/uboot-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
PKG_DOWNLOAD ?= wget -c '$(strip ${PKG_URL})' -O ${PKG_FILE}
SRC_PARENT_DIR ?= ./_tmp_
SRC_ROOT_DIR ?= $(shell \
    [ -f ${PKG_FILE} -o -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (${PKG_DOWNLOAD}) >&2; \
    [ -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (mkdir -p ${SRC_PARENT_DIR}; ${UNCOMPRESS} ${PKG_FILE} -C ${SRC_PARENT_DIR}) >&2; \
    ls -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) \
)
INSTALL_DIR ?= ${HOME}/tftpd
INSTALL_CMD ?= [ -d ${INSTALL_DIR} ] || mkdir -p ${INSTALL_DIR}; ${CP} ${SRC_ROOT_DIR}/u-boot* ${INSTALL_DIR}/
UNINSTALL_CMD ?= ls ${INSTALL_DIR}/u-boot* | grep -v u-boot.mk | xargs -I {} rm {}
DEFCONFIG ?= configs/mx6ull_14x14_evk_nand_defconfig
EXT_TARGETS +=
CUSTOM_FILES += ${DEFCONFIG}
__CUSTOMIZED_DEPENDENCIES := $(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i})

$(foreach i, DEFCONFIG EXT_TARGETS, $(eval ${i} := $(strip ${${i}})))
$(foreach i, EXT_TARGETS CUSTOM_FILES, $(eval ${i} := $(sort ${${i}}))) # To filter out repeated items

.PHONY: all menuconfig $(if ${DEFCONFIG},defconfig) \
    download clean distclean install uninstall ${EXT_TARGETS} help showvars

all: ${__CUSTOMIZED_DEPENDENCIES}
	${MAKE} -C ${SRC_ROOT_DIR} ${MAKE_ARGS} -j ${NTHREADS}

menuconfig: $(if ${DEFCONFIG},defconfig) ${__CUSTOMIZED_DEPENDENCIES}
	[ -z "${DEFCONFIG}" ] && conf_file=.config || conf_file=${DEFCONFIG}; \
	[ "$${conf_file}" = ".config" -a -f $${conf_file} ] && ${CP} $${conf_file} ${SRC_ROOT_DIR}/.config || : ; \
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
	[ -f ${PKG_FILE} ] || (${PKG_DOWNLOAD})

install:
	${INSTALL_CMD}

uninstall:
	${UNINSTALL_CMD}

${EXT_TARGETS}: ${__CUSTOMIZED_DEPENDENCIES}

clean distclean ${EXT_TARGETS}: %:
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

ifeq (1,1)

define custom_file_rule
${SRC_ROOT_DIR}/${1}: ${1}
	${DIFF} ${SRC_ROOT_DIR}/${1} ${1} && ${TOUCH} ${SRC_ROOT_DIR}/${1} || ${CP} ${1} ${SRC_ROOT_DIR}/${1}
endef

$(foreach i, ${CUSTOM_FILES}, $(eval $(call custom_file_rule,${i})))

else # This works as well, but it's more complecated for writing.

NULL :=
\t := ${NULL}	${NULL}
define \n


endef

$(foreach i, ${CUSTOM_FILES}, \
    $(eval \
        ${SRC_ROOT_DIR}/${i}: ${i} \
            ${\n}${\t}${DIFF} ${SRC_ROOT_DIR}/${i} ${i} && ${TOUCH} ${SRC_ROOT_DIR}/${i} || ${CP} ${i} ${SRC_ROOT_DIR}/${i} \
    ) \
)

endif

# Failure case (1): target 'xxx' doesn't match the target pattern
#${CUSTOM_FILES}: ${SRC_ROOT_DIR}/%: %
# Failure case (2): substitution failure which leads to circular dependency
#${__CUSTOMIZED_DEPENDENCIES}: %: $(subst ${SRC_ROOT_DIR}/,,%)
#	${DIFF} $@ $< && ${TOUCH} $@ || ${CP} $< $@ # Shared by (1) and (2)

# Failure case (3): never triggered
#${__CUSTOMIZED_DEPENDENCIES}: %:
# Failure case (4): almost a success except for the thundering herd problem
#${__CUSTOMIZED_DEPENDENCIES}: %: ${CUSTOM_FILES}
#	${DIFF} $@ ${@:${SRC_ROOT_DIR}/%=%} && ${TOUCH} $@ || ${CP} ${@:${SRC_ROOT_DIR}/%=%} $@ # Shared by (3) and (4)

help:
	@echo "Core directives:"
	@echo "  1. ${MAKE} download   - Download the source package manually; Usually unnecessary"
	@echo "  2. ${MAKE} showvars   - Display customized variables and their values"
	@echo "  3. ${MAKE} menuconfig - Interactive configuration (automatically saved if changed)"
	@echo "  4. ${MAKE}            - Build U-Boot in a default way"
	@echo "  5. ${MAKE} clean      - Clean most generated files and directories"
	@echo "  6. ${MAKE} distclean  - Clean all generated files and directories (including .config)"
	@echo "  7. ${MAKE} install    - Copy the generated u-boot* files to the directory specified by INSTALL_DIR"
	@echo "  8. ${MAKE} uninstall  - Delete u-boot* files in the directory specified by INSTALL_DIR"
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

__VARS__ := ARCH CROSS_COMPILE __VER__ MAKE_ARGS NTHREADS \
    CP DIFF TOUCH UNCOMPRESS \
    PKG_FILE PKG_URL PKG_DOWNLOAD SRC_PARENT_DIR SRC_ROOT_DIR \
    INSTALL_DIR INSTALL_CMD UNINSTALL_CMD \
    DEFCONFIG EXT_TARGETS CUSTOM_FILES USER_HELP_PRINTS

showvars:
	$(info )
	$(foreach i, ${__VARS__}, $(info ${i} = ${$i}) $(info ))
	@:

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
#   04. Support multiple source directories with different versions.
#
# >>> 2024-02-02, Man Hung-Coeng <udc577@126.com>:
#   01. Add "install" and "uninstall" targets.
#
# >>> 2024-02-04, Man Hung-Coeng <udc577@126.com>:
#   01. Remove --preserve and --update options of CP definition
#       to avoid old timestamps and unexpected skipping.
#   02. Update the help info.
#   03. Change the default value of INSTALL_DIR from ${PWD} to ${HOME}/tftpd.
#
# >>> 2024-02-14, Man Hung-Coeng <udc577@126.com>:
#   01. Change the non-error output redirection of Shell commands of
#       SRC_ROOT_DIR definition from /dev/null to stderr.
#   02. Beautify the display of extra directive(s) of "make help".
#   03. Rename variable SRC_PKG_* to PKG_*.
#   04. Make EXTRA_TARGETS depend on CUSTOM_FILES.
#
# >>> 2024-02-18, Man Hung-Coeng <udc577@126.com>:
#   01. Make target "menuconfig" depend on CUSTOM_FILES.
#
# >>> 2024-03-02, Man Hung-Coeng <udc577@126.com>:
#   01. Fix an unexpected-operator syntax error in rules of target "menuconfig",
#       which is caused by misuse of double-equals sign (==).
#   02. Support customizing number of worker threads for the compilation of
#       default target through NTHREADS environment variable.
#   03. Change the assignment operator of EXTRA_TARGETS (new name: EXT_TARGETS)
#       and CUSTOM_FILES to addition assignment operator (+=).
#   04. Add a new target "showvars".
#   05. Enhance "make help" by allowing to define extra printing commands.
#

