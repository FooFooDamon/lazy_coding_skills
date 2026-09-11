# SPDX-License-Identifier: Apache-2.0

#
# Rules for initializing an STM32CubeIDE project.
# Initially generated from:
#	vim/templates/mk.tpl.list/stm32_init.tpl
# of:
#	https://github.com/FooFooDamon/lazy_coding_skills
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#

GEN_FILE=Core/Inc/generated.h

all: ../${GEN_FILE}

../${GEN_FILE}: .FORCE
	@echo ""
	@:
	@if [ ! -e $@ ]; then \
		( \
			echo "/*"; \
			echo " * Automatically-generated file. Do not edit!"; \
			echo " */"; \
			echo ""; \
			echo "#ifndef __GENERATED_H__"; \
			echo "#define __GENERATED_H__"; \
			echo ""; \
			echo "#define __REVISION__ \"<none>\""; \
			echo ""; \
			echo "#define USER_VECT_TAB_ADDRESS"; \
			echo "#define VECT_TAB_SRAM"; \
			echo ""; \
			echo "#endif /* __GENERATED_H__ */"; \
		) > $@; \
		echo "Finished generating: ${GEN_FILE}"; \
	fi
	@:
	@REVISION=$$(git -C .. log --abbrev-commit --abbrev=8 --pretty=format:%h -n 1 . 2> /dev/null); \
	DIRTY_FLAG=$$([ -z $$(git -C .. diff --name-only . 2> /dev/null | head -n 1) ] && echo "" || echo "-dirty"); \
	FULL_REV=$${REVISION}$${DIRTY_FLAG}; \
	[ -n "$${FULL_REV}" ] || FULL_REV="<none>"; \
	CURR_REV=$$(grep "^#define __REVISION__" $@ | awk '{ print $$3 }' | sed 's/"\([^"]\+\)"/\1/'); \
	if [ "$${CURR_REV}" != "$${FULL_REV}" ]; then \
		sed -i "s/^\(#define __REVISION__\)[ \t]\+.\+/\1 \"$${FULL_REV}\"/" $@; \
		echo "Finished updating macro __REVISION__: $${CURR_REV} -> $${FULL_REV}"; \
	fi
	@:
	@LNK_SCRIPT_CNT=$$(grep -c -E "(STM32[a-zA-Z0-9]+_RAM.ld)" ../.cproject); \
	if [ -z "$${LNK_SCRIPT_CNT}" -o $${LNK_SCRIPT_CNT} -eq 0 ]; then \
		for i in USER_VECT_TAB_ADDRESS VECT_TAB_SRAM; \
		do \
			if [ $$(grep -c "^#define[ ]\+$${i}\>" $@) -gt 0 ]; then \
				sed -i "s/\(#define[ ]\+$${i}\>\)/\/\* \1 \*\//" $@; \
				echo "Finished commenting macro: $${i}"; \
			fi; \
		done; \
	else \
		for i in USER_VECT_TAB_ADDRESS VECT_TAB_SRAM; \
		do \
			if [ $$(grep -c "^/[/*][ ]*#define[ ]\+$${i}\>" $@) -gt 0 ]; then \
				sed -i "s/\(\/[/*]\)[ ]*\(#define[ ]\+$${i}\>\)[ ]*\([/*]\/\)*/\2/" $@; \
				echo "Finished UNcommenting macro: $${i}"; \
			fi; \
		done; \
	fi
	@:
	@echo ""

.FORCE:
