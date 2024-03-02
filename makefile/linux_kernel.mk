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
CP ?= cp -R -P
DIFF ?= diff --color
TOUCH ?= touch
UNCOMPRESS ?= tar -zxvf
PKG_FILE ?= ./linux-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
# Rule of URL: https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives
PKG_URL ?= https://github.com/nxp-imx/linux-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
PKG_DOWNLOAD ?= wget -c '$(strip ${PKG_URL})' -O ${PKG_FILE}
SRC_PARENT_DIR ?= ./_tmp_
SRC_ROOT_DIR ?= $(shell \
    [ -f ${PKG_FILE} -o -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (${PKG_DOWNLOAD}) >&2; \
    [ -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) ] || (mkdir -p ${SRC_PARENT_DIR}; ${UNCOMPRESS} ${PKG_FILE} -C ${SRC_PARENT_DIR}) >&2; \
    ls -d ${SRC_PARENT_DIR}/$(notdir ${PKG_FILE:.tar.gz=}) \
)
KERNEL_IMAGE ?= $(if ${KBUILD_IMAGE},${KBUILD_IMAGE},zImage)
DTS_PATH ?= arch/arm/boot/dts/imx6ull-14x14-evk.dts
__DTB_PATH := $(strip ${DTS_PATH:%.dts=%.dtb})
INSTALL_DIR ?= ${HOME}/tftpd
INSTALL_CMD ?= [ -d ${INSTALL_DIR} ] || mkdir -p ${INSTALL_DIR}; \
    find ${SRC_ROOT_DIR}/arch/${ARCH}/boot/ -name ${KERNEL_IMAGE} -exec ${CP} {} ${INSTALL_DIR}/ \; \
    $(if ${__DTB_PATH},; ${CP} ${SRC_ROOT_DIR}/${__DTB_PATH} ${INSTALL_DIR}/)
UNINSTALL_CMD ?= rm -f ${INSTALL_DIR}/${KERNEL_IMAGE} $(if ${__DTB_PATH},${INSTALL_DIR}/$(notdir ${__DTB_PATH}))
LOCALVERSION ?= $(if $(wildcard .localversion),-$(shell cat .localversion))
__VER__ ?= 123456789abc
KBUILD_BUILD_VERSION ?= $(if ${BUILDVERSION},${BUILDVERSION},${__VER__})
KCFLAGS ?= -DUTS_NODENAME=\\\"`hostname`[${__VER__}]\\\" -DUTS_DOMAINNAME=\\\"${VCS}://ver.${__VER__}.nil/\\\"
MAKE_ARGS ?= ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} $(if ${__VER__},KCFLAGS="${KCFLAGS}") \
    KBUILD_BUILD_VERSION=${KBUILD_BUILD_VERSION} $(if ${LOCALVERSION},LOCALVERSION=${LOCALVERSION})
NTHREADS ?= $(shell nproc)
DEFCONFIG ?= arch/arm/configs/imx_v7_defconfig
EXT_TARGETS += $(if ${__DTB_PATH},dtbs dtbs_install) modules_install \
    deb-pkg bindeb-pkg tar-pkg targz-pkg tarbz2-pkg tarxz-pkg rpm-pkg binrpm-pkg
__PARTIAL_MODULES := $(filter %.ko, ${EXT_TARGETS})
EXT_TARGETS += $(if ${__PARTIAL_MODULES},mods_install)
CUSTOM_FILES += ${DEFCONFIG} ${DTS_PATH}
__CUSTOMIZED_DEPENDENCIES := $(foreach i, ${CUSTOM_FILES}, ${SRC_ROOT_DIR}/${i})

$(foreach i, INSTALL_DIR DEFCONFIG EXT_TARGETS, $(eval ${i} := $(strip ${${i}})))
$(foreach i, EXT_TARGETS CUSTOM_FILES, $(eval ${i} := $(sort ${${i}}))) # To filter out repeated items

.PHONY: ${KERNEL_IMAGE} all $(if ${__DTB_PATH},dtb) modules menuconfig $(if ${DEFCONFIG},defconfig) \
    download clean distclean install uninstall ${EXT_TARGETS} help precheck showvars

${KERNEL_IMAGE} all modules: ${__CUSTOMIZED_DEPENDENCIES} precheck
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS} -j ${NTHREADS}

ifneq (${__DTB_PATH},)

dtb: ${__CUSTOMIZED_DEPENDENCIES}
	${MAKE} $(notdir ${__DTB_PATH}) -C ${SRC_ROOT_DIR} ${MAKE_ARGS} || ( \
		printf "\n*** Run: ${MAKE} dtbs%s\n*** If the error is: No rule to make target '${__DTB_PATH}'\n\n" \
			"$(if $(filter dtbs, ${EXT_TARGETS}),, -C ${PWD}/${SRC_ROOT_DIR})" >&2 ; \
		false \
	)

endif

menuconfig: $(if ${DEFCONFIG},defconfig) ${__CUSTOMIZED_DEPENDENCIES}
	[ -z "${DEFCONFIG}" ] && conf_file=.config || conf_file=${DEFCONFIG}; \
	[ "$${conf_file}" = ".config" -a -f $${conf_file} ] && ${CP} $${conf_file} ${SRC_ROOT_DIR}/.config || : ; \
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
	[ -f ${PKG_FILE} ] || (${PKG_DOWNLOAD})

install:
	if [ -n "$(shell echo ${INSTALL_DIR} | grep '^/boot$$\|^/boot/[^/]*')" ]; then \
		${MAKE} install INSTALL_PATH=${INSTALL_DIR} -C ${SRC_ROOT_DIR} ${MAKE_ARGS}; \
	else \
		${INSTALL_CMD}; \
	fi

uninstall:
	if [ -n "$(shell echo ${INSTALL_DIR} | grep '^/boot$$\|^/boot/[^/]*')" ]; then \
		echo "*** Uninstallation not supported" >&2; \
		false; \
	else \
		${UNINSTALL_CMD}; \
	fi

${EXT_TARGETS}: ${__CUSTOMIZED_DEPENDENCIES} precheck

$(filter %-pkg, ${EXT_TARGETS}): %:
	cd ${SRC_ROOT_DIR}; \
	$(if ${KBUILD_BUILD_USER},export NAME=${KBUILD_BUILD_USER};) \
	${MAKE} $@ ${MAKE_ARGS} BRANCH=$(if ${BRANCH},${BRANCH},default) \
		KBUILD_DEBARCH=$(if ${KBUILD_DEBARCH},${KBUILD_DEBARCH},${ARCH}) \
		KDEB_PKGVERSION=$$(${MAKE} kernelversion -s | grep "^[0-9]\+")${LOCALVERSION}-${KBUILD_BUILD_VERSION} \
		-j ${NTHREADS}

ifneq (${__PARTIAL_MODULES},)

.PHONY: mods ${__PARTIAL_MODULES:%.ko=%.ko-install}

${__PARTIAL_MODULES}: %: # ${PWD}/${SRC_ROOT_DIR}/%
	${MAKE} modules modules='$@' -C ${SRC_ROOT_DIR} ${MAKE_ARGS} -j ${NTHREADS}

${__PARTIAL_MODULES:%.ko=%.ko-install}: %.ko-install: ${PWD}/${SRC_ROOT_DIR}/%.ko
	${MAKE} modules_install modules='$(subst ${PWD}/${SRC_ROOT_DIR}/,,$<)' -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

mods:
	${MAKE} modules modules='${__PARTIAL_MODULES}' -C ${SRC_ROOT_DIR} ${MAKE_ARGS} -j ${NTHREADS}

mods_install:
	${MAKE} modules_install modules='${__PARTIAL_MODULES}' -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

define default_module_target_alias
${1}: ${2}
install_${1}: ${2}-install
endef

APPLY_DEFAULT_MODULE_TARGET_ALIAS = $(foreach i, ${__PARTIAL_MODULES}, \
    $(eval \
        $(call default_module_target_alias,$(notdir ${i:.ko=}),${i}) \
    ) \
)

DEFAULT_USER_HELP_PRINTS ?= \
    for i in $(subst .ko,,$(notdir ${__PARTIAL_MODULES})); \
    do \
        echo "  * ${MAKE} $${i}"; \
        echo "  * ${MAKE} install_$${i}"; \
    done;

endif # ifneq (${__PARTIAL_MODULES},)

clean distclean $(filter-out %-pkg %.ko mods mods_install, ${EXT_TARGETS}): %:
	${MAKE} $@ -C ${SRC_ROOT_DIR} ${MAKE_ARGS}

define custom_file_rule
${SRC_ROOT_DIR}/${1}: ${1}
	${DIFF} ${SRC_ROOT_DIR}/${1} ${1} && ${TOUCH} ${SRC_ROOT_DIR}/${1} || ${CP} ${1} ${SRC_ROOT_DIR}/${1}
endef

$(foreach i, ${CUSTOM_FILES}, $(eval $(call custom_file_rule,${i})))

help: precheck
	@echo "Core directives:"
	@echo "   1. ${MAKE} download   - Download the source package manually; Usually unnecessary"
	@echo "   2. ${MAKE} showvars   - Display customized variables and their values"
	@echo "   3. ${MAKE} menuconfig - Interactive configuration (automatically saved if changed)"
	@echo "   4. ${MAKE} [${KERNEL_IMAGE}]   - Build the kernel image (with or without \"${KERNEL_IMAGE}\")"
	@echo "   5. ${MAKE} all        - Build all necessary stuff"
	@echo "   6. ${MAKE} dtb        - $(if ${__DTB_PATH},Build ${__DTB_PATH},UNAVAILABLE)"
	@echo "   7. ${MAKE} modules [modules='relative/path/to/mod1.ko relative/path/to/mod2.ko ...']"
	@echo "                      - Build $(if ${__PARTIAL_MODULES},*ALL*,all) enabled modules, or those specified in \"modules\" list"
	$(if ${__PARTIAL_MODULES},@echo "  7b. ${MAKE} mods       - Build *PARTIAL* enabled modules specified in extended targets")
	@echo "   8. ${MAKE} clean      - Clean most generated files and directories"
	@echo "   9. ${MAKE} distclean  - Clean all generated files and directories (including .config)"
ifneq ($(shell echo ${INSTALL_DIR} | grep '^/boot$$\|^/boot/[^/]*'),)
	@echo "  10. ${MAKE} install    - Install ${KERNEL_IMAGE} to ${INSTALL_DIR}"
	@echo "  11. ${MAKE} uninstall  - UNAVAILABLE"
else
	@echo "  10. ${MAKE} install    - Copy ${KERNEL_IMAGE} $(if ${__DTB_PATH},and $(notdir ${__DTB_PATH})) to the directory specified by INSTALL_DIR"
	@echo "  11. ${MAKE} uninstall  - Delete ${KERNEL_IMAGE} $(if ${__DTB_PATH},and $(notdir ${__DTB_PATH})) in the directory specified by INSTALL_DIR"
endif
	@echo ""
	@printf "Extended directive(s): "
ifeq (${EXT_TARGETS},)
	@echo "None"
else
	@for i in $(sort ${EXT_TARGETS} ${__PARTIAL_MODULES:%.ko=%.ko-install}); \
	do \
		printf "\n  * ${MAKE} $${i}"; \
		[ "$${i}" = "modules_install" ] && printf " [modules='relative/path/to/mod1.ko relative/path/to/mod2.ko ...']" || :; \
	done
	@printf "\n  --"
	@printf "\n  Run \"${MAKE} help -C ${PWD}/${SRC_ROOT_DIR}\""
	@printf "\n  to see detailed descriptions (except for mods_install and *.ko-install)."
	@printf "\n"
endif
	$(if $(strip ${USER_HELP_PRINTS}),@printf "\nUser help info:\n"; (${USER_HELP_PRINTS}))

precheck:
	@if [ -z "${KBUILD_BUILD_USER}" -o -z "${EMAIL}" ]; then \
		echo "----"; \
		echo; \
		if [ -z "${KBUILD_BUILD_USER}" ]; then \
			echo "*** KBUILD_BUILD_USER unspecified!"; \
			echo "Some default value will be used as the maintainer's name, which may not be what you want!"; \
			echo "To avoid this, consider defining it in your Makefile or through command line like:"; \
			echo "  export KBUILD_BUILD_USER=YourNameWithoutBlankChars"; \
			echo; \
		fi; \
		if [ -z "${EMAIL}" ]; then \
			echo "*** EMAIL unspecified!"; \
			echo "Some default value will be used as the E-Mail address, which may not be what you want!"; \
			echo "To avoid this, consider defining it in your Makefile or through command line like:"; \
			echo "  export EMAIL=xxx@gmail.com"; \
			echo; \
		fi; \
		echo "----"; \
	fi >&2

__VARS__ := ARCH CROSS_COMPILE CP DIFF TOUCH UNCOMPRESS \
    PKG_FILE PKG_URL PKG_DOWNLOAD SRC_PARENT_DIR SRC_ROOT_DIR \
    KBUILD_IMAGE KERNEL_IMAGE DTS_PATH INSTALL_DTBS_PATH \
    INSTALL_DIR INSTALL_CMD UNINSTALL_CMD \
    LOCALVERSION BUILDVERSION __VER__ KBUILD_BUILD_VERSION MAKE_ARGS NTHREADS \
    DEFCONFIG EXT_TARGETS CUSTOM_FILES DEFAULT_USER_HELP_PRINTS USER_HELP_PRINTS

showvars:
	$(info )
	$(foreach i, ${__VARS__}, $(info ${i} = ${$i}) $(info ))
	@#echo "----" >&2
	@#echo "Note: Values containing quotes may not be displayed correctly." >&2
	@#echo "----" >&2

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
# >>> 2024-03-02, Man Hung-Coeng <udc577@126.com>:
#   01. Fix an unexpected-operator syntax error in rules of target "menuconfig",
#       which is caused by misuse of double-equals sign (==).
#   02. Support customizing number of worker threads for the compilation of
#       kernel image through NTHREADS environment variable.
#   03. Change the assignment operator of EXTRA_TARGETS (new name: EXT_TARGETS)
#       and CUSTOM_FILES to addition assignment operator (+=).
#   04. Add new targets: dtb, dtbs_install, mod[ule]s, mod[ule]s_install, *-pkg
#       and showvars.
#   05. Enhance "make help" by allowing to define extra printing commands.
#

