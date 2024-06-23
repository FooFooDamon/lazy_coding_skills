# SPDX-License-Identifier: GPL-2.0

#
# Makefile wrapper for U-Boot porting project.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
#

override undefine LAZY_CODING_MAKEFILES
LAZY_CODING_MAKEFILES := __ver__.mk u-boot.mk

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES}, -a -s ${i}) ] && echo 1 || echo 0),0)

.PHONY: all prepare

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
ARCH := arm
CROSS_COMPILE := arm-linux-gnueabihf-
PKG_FILE ?= ./uboot-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
PKG_URL ?= https://github.com/nxp-imx/uboot-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
INSTALL_DIR ?= ${HOME}/tftpd/imx6ullevk
DEFCONFIG := configs/mx6ull_14x14_evk_nand_defconfig
EXT_TARGETS +=
CUSTOM_FILES += arch/${ARCH}/cpu/armv7/start.S \
    arch/${ARCH}/cpu/armv7/soc.c \
    include/configs/mx6ullevk.h \
    board/freescale/mx6ullevk/mx6ullevk.c \
    drivers/net/phy/ti.c

include $(word 2, ${LAZY_CODING_MAKEFILES})

# FIXME: Add more rules if needed, and delete this comment line then.

endif
