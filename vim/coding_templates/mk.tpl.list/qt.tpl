# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#

.PHONY: all prepare dependencies

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

override undefine PREREQUISITE_FILES
PREREQUISITE_FILES := ${LAZY_CODING_URL}/raw/main/makefile/__ver__.mk \
    ${LAZY_CODING_URL}/raw/main/c_and_cpp/qt/qt_print.hpp \
    ${LAZY_CODING_URL}/raw/77d444b429e8ac0166e876834a52e97e0d3b3237/c_and_cpp/native/signal_handling.c \
    ${LAZY_CODING_URL}/raw/77d444b429e8ac0166e876834a52e97e0d3b3237/c_and_cpp/native/signal_handling.h \

ifeq ($(shell [ -s QtMakefile $(foreach i, $(notdir ${PREREQUISITE_FILES}), -a -s ${i}) ] && echo 1 || echo 0),0)

all prepare: dependencies
	@for i in ${PREREQUISITE_FILES}; \
	do \
		[ -s $$(basename $${i}) ] || wget -c -O $$(basename $${i}) "$${i}"; \
	done
	@[ -s QtMakefile ] || qmake -o QtMakefile
	@echo "~ ~ ~ Minimum preparation finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

all: dependencies

include __ver__.mk
include QtMakefile

DEFINES += -D__VER__=\"${__VER__}\"
CFLAGS += -Wno-unused-parameter
CXXFLAGS += -Wno-unused-parameter
INCPATH +=
LFLAGS +=
LIBS +=
NO_CPPCHECK = true
OS_MACRO ?= -D__linux__
PREDEFS_FOR_CPPCHECK ?= $(if $(wildcard moc_predefs.h), --include=moc_predefs.h, \
    ${OS_MACRO} $$(g++ -dM -E - < /dev/null | grep ENDIAN | awk '{ printf("-D%s=%s\n", $$2, $$3) }'))

.PHONY: ext_clean check debug_patch ui_fix

clean: ext_clean

ext_clean:
	rm -f ${TARGET} *.d *.plist

check:
	-${NO_CPPCHECK} \
		|| cppcheck --quiet --force --enable=all --suppress=missingIncludeSystem \
		-j $$(nproc) --language=c --std=c11 \
		${PREDEFS_FOR_CPPCHECK} \
		${DEFINES} ${INCPATH} $(filter %.c, ${SOURCES})
	-${NO_CPPCHECK} && printf "\n\033[0;33m[Warning] Cppcheck has been disabled since it consumes too much time!\033[0m\n%s\n\n" \
		"If you want to enable it, run with NO_CPPCHECK=false" >&2 \
		|| cppcheck --quiet --force --enable=all --suppress=missingIncludeSystem \
		-j $$(nproc) --language=c++ --std=c++11 \
		--library=qt ${PREDEFS_FOR_CPPCHECK} \
		${DEFINES} ${INCPATH} $(filter-out %.c moc_%.cpp, ${SOURCES})
	clang --analyze $(filter %.c, ${SOURCES}) ${CFLAGS} ${INCPATH}
	clang --analyze $(filter-out %.c moc_%.cpp, ${SOURCES}) ${CXXFLAGS} ${INCPATH}

debug_patch:
	@if [ -s ${TARGET}.debug.pri ]; then \
		echo "${TARGET}.debug.pri already exists." >&2; \
	else \
		echo "CONFIG += debug" > ${TARGET}.debug.pri; \
		${MAKE} distclean; \
		printf "\n~ ~ ~ ${TARGET}.debug.pri was generated successfully. ~ ~ ~\n\nYou can re-build the project now.\n\n"; \
	fi

ui_fix: ui_${TARGET}.h
	sed -i 's/\(.*\<QPalette::PlaceholderText\>.*\)/\/\/\1/' $<

endif

export DEPENDENCY_DIRS ?= $(abspath ../3rdparty)

dependencies:
	@for i in ${DEPENDENCY_DIRS}; \
	do \
		[ -s $${i}/[Mm]akefile ] && ${MAKE} $(filter all prepare, ${MAKECMDGOALS}) -C $${i} || true; \
	done
