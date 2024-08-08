# SPDX-License-Identifier: GPL-2.0

#
# Makefile wrapper for root filesystem making project based on Buildroot.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
#

override undefine LAZY_CODING_MAKEFILES
LAZY_CODING_MAKEFILES := __ver__.mk buildroot.mk

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES}, -a -s ${i}) ] && echo 1 || echo 0),0)

.PHONY: all help prepare

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

include $(word 1, ${LAZY_CODING_MAKEFILES})

#
# FIXME: Modify variables below according to your needs and delete this line then.
#
PKG_VERSION ?= 2023.02
INSTALL_DIR ?= ${HOME}/tftpd/imx6ullevk
EXT_TARGETS +=
CUSTOM_FILES += package/busybox/0005-libbb-printable_string2.patch \
    package/busybox/0006-libbb-unicode_conv_to_printable2.patch

include $(word 2, ${LAZY_CODING_MAKEFILES})

# FIXME: Add more rules if needed, and delete this comment line then.

endif
