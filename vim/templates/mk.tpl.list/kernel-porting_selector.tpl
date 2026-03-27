# SPDX-License-Identifier: GPL-2.0

#
# Selector of multiple-version Linux kernel porting project.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
#

$(foreach i, __VER_LIST __VER_COUNT, $(eval override undefine ${i}))
__VER_LIST := $(wildcard [0-9]*.[0-9]*)
__VER_COUNT := $(words ${__VER_LIST})
ifeq (${__VER_COUNT},0)
    $(error No available versions)
else ifeq (${__VER_COUNT},1)
    override undefine KVER
    #KVER := $(firstword ${__VER_LIST})
    KVER := ${__VER_LIST}
else
    __VER_FILE ?= .which
    ifneq ($(wildcard ${__VER_FILE}),)
        KVER ?= $(file < ${__VER_FILE})
    else
        # FIXME: Using "?=" here will cause 3 times of assignment, why?!
        KVER := $(shell \
            echo "Found ${__VER_COUNT} versions:" >&2; \
            for i in ${__VER_LIST}; do echo "  $${i}" >&2; done; \
            read -p "Input one of the versions above, then press Enter: " KVER && echo $${KVER} | tee ${__VER_FILE}; \
            printf "Your choice has been saved to file: ${__VER_FILE}\nYou can modify it anytime you want.\n" >&2; \
        )
    endif
endif
KERNEL_IMAGE ?= Image
CORE_TARGETS := $(shell ${MAKE} -C ${KVER} help -s | sed -n "1,/^Extended directive/p" | awk '{ print $$3 }' | grep -v '^[-[*{<]')
EXT_TARGETS := $(shell ${MAKE} -C ${KVER} showvars -s | grep EXT_TARGETS | awk -F = '{ print $$2 }')
__MODULES := $(filter %.ko, ${EXT_TARGETS})
EXT_TARGETS += ${__MODULES:.ko=.ko-install}
USER_TARGETS := $(foreach i, $(notdir ${__MODULES:.ko=}), ${i} install_${i})

${KERNEL_IMAGE} ${CORE_TARGETS} ${EXT_TARGETS} ${USER_TARGETS} help: %:
	${MAKE} $@ -C ${KVER}

PRIV_MAKEFILES := $(filter-out default.priv.mk,$(wildcard *.priv.mk))
-include $(if ${PRIV_MAKEFILES}, ${PRIV_MAKEFILES}, <placeholder>.mk)

