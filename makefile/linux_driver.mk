#
# Makefile template for Linux driver.
#
# Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
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

#
# >>>>>>>> USAGE <<<<<<<<
#
# Including this file in your own makefile and doing some customizations
# is preferable to modifing this file directly.
#
# Example 1 (Simplest; Usually for tiny tests or practices):
# (1) Assume you have multiple tests like:
#         /path/to/test/root/directory
#             |-- test1
#             |       `-- test1.c
#             |-- test2
#             |       |-- test2.c
#             |       `-- test2_app.c
#             .		.
#             .		.
#             .		.
#             `-- testN
#                     `-- testN.c
# (2) Put this file in ${HOME}/src directory, or anywhere you want.
#     The path to Linux kernel source code directory is ${HOME}/src/linux,
#     if not, make a symbolic link.
# (3) If you're an ARM driver developer, you don't even need to write a makefile,
# 	  just making a symbolic link is enough, take test1 as example:
#         $ ln -s ${HOME}/src/linux_driver.mk /path/to/test1/Makefile
# (3') If you're developing drivers of other platforms, e.g. X86,
# 	  a little more jobs need to be done.
# 	  First, create a file with a path of ${HOME}/src/x86_driver.mk
# 	  and with content as below:
# 	  	  STRIP := strip
# 	  	  APP_CC := gcc
# 	  	  include ${HOME}/src/linux_driver.mk
# 	  Then, make a symbolic link for each test case, take test1 as example:
#         $ ln -s ${HOME}/src/x86_driver.mk /path/to/test1/Makefile
#
# Example 2 (A bit complicated; Usually for formal projects):
# (1) Assume you have a formal project like:
#         /path/to/project/source/code/directory
#             |-- common
#             |       `-- __ver__.mk # your definition of __VER__
#             |-- app
#             |-- kernel # linux kernel source code
#             `-- drivers
#                     |-- driver1
#                     |       |-- driver1_main.c
#                     |       |-- driver1_utils.c
#                     |       |-- driver1_app_main.c
#                     |       |-- driver1_app_utils.c
#                     |       `-- Makefile
#                     |-- driver2 # containing stuff similar to driver1
#                     .		.
#                     .		.
#                     .		.
#                     `-- driverN # containing stuff similar to driver1
# (2) Put this file in the common directory.
# (3) Edit the makefile of each driver according to the actual situation,
#     let's take driver1 as example, it's makefile may be like:
#         KERNEL_ROOT := ${PWD}/../../kernel
#         DRVNAME := driver1
#         ${DRVNAME}-objs := driver1_main.o driver1_utils.o
#         APP_NAME := driver1_app
#         APP_OBJS := driver1_app_main.o driver1_app_utils.o
#         # Other settings if needed: STRIP, APP_CC, APP_DEFINES, etc.
#         include ${PWD}/../../common/__ver__.mk
#         include ${PWD}/../../common/linux_driver.mk
#

#=======================
# Global settings.
#=======================
STRIP ?= arm-linux-gnueabihf-strip
NDEBUG ?= y

#=======================
# For device driver.
#=======================
KERNEL_ROOT ?= ${HOME}/src/linux # Or set it to another path in your own makefile if you have a different one.
ifeq (${DRVNAME},) # Define it explicitly in your own makefile if there're multiple source files for this driver.
    export DRVNAME := $(basename $(notdir $(shell find ./ -name "*.c" | grep -v '_app\.c$$' | head -n 1)))
    ifeq (${DRVNAME},)
        $(error DRVNAME not specified and not deductive)
    endif
endif
obj-m := ${DRVNAME}.o
# CFLAGS is not permitted here, otherwise an error will be triggered with a message below:
# *** CFLAGS was changed in "...". Fix it to use ccflags-y.
ccflags-y += -D__VER__=\"${__VER__}\" # Define the version number in another makefile, or just ignore it.
ifeq (${NDEBUG},)
    ccflags-y += -O0 -g
endif

#=======================
# For application demo.
#=======================
APP_NAME ?= $(basename $(shell find ./ -name "*.c" | grep '_app\.c$$' | head -n 1))
APP_OBJS ?= ${APP_NAME}.o
# CC is not suitable here, because it has a default value of "cc" which mean it's not empty.
APP_CC ?= arm-linux-gnueabihf-gcc
ifeq (${NDEBUG},)
    APP_DEBUG_FLAGS ?= -O0 -g
else
    APP_DEBUG_FLAGS ?= -O2 -DNDEBUG
endif
APP_CFLAGS ?= -D_REENTRANT -D__VER__=\"${__VER__}\" -fPIC -Wall -Werror -ansi -Wpedantic \
    -W -Wno-variadic-macros -Wno-unused-parameter -Wno-missing-field-initializers \
    -Wno-implicit-fallthrough ${APP_DEBUG_FLAGS} ${APP_DEFINES} ${APP_INCLUDES} \
    ${OTHER_APP_CFLAGS}

.PHONY: all debug clean

all: ${PREREQUISITES} ${DRVNAME}.ko ${APP_NAME}

${DRVNAME}.ko: ${obj-m:.o=.c} ${${DRVNAME}-objs:.o=.c}
	make -C ${KERNEL_ROOT} M=`pwd` modules # Also includes the generation of .o files.
	[ -f $@ ] || mv $$(ls *.ko | head -n 1) $@
	[ -z "${NDEBUG}" ] || ${STRIP} -d $@

${APP_NAME}: ${APP_OBJS}
	${APP_CC} -o $@ -fPIE $^ ${APP_LDFLAGS}
	[ -z "${NDEBUG}" ] || ${STRIP} -s $@

D_FILES := $(foreach i, ${APP_OBJS:.o=.c}, ${i}.d)
CMD_FILES := $(shell find . -name ".*.o.cmd" | grep -v "\.mod\.o\.cmd")

# Dependencies for auto-detection of header content update.
-include ${D_FILES}
ifneq (${CMD_FILES},)
    # TODO: This does not work! Why?
    include ${CMD_FILES}
endif

%.o: %.c # This only affects application object files. See the rule of ${DRVNAME}.ko above.
	${APP_CC} -Wp,-MD,$<.d ${APP_CFLAGS} -c -o $@ $<

debug:
	make NDEBUG=""

clean:
	rm -f ${APP_NAME} ${APP_OBJS} ${D_FILES}
	make -C ${KERNEL_ROOT} M=`pwd` clean # modname=${DRVNAME}

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-04-07, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2023-04-08, Man Hung-Coeng <udc577@126.com>:
#   01. Fix the bug of ${DRVNAME}.ko not recompiling while it's sources are changed.
#
# >>> 2023-04-16, Man Hung-Coeng <udc577@126.com>:
#   01. Add ${PREREQUISITES} to "all" target to make it possible
#   	to make some optional preparations before compilation.
#   02. Specify target "all", "debug" and "clean" as ".PHONY".
#   03. Add dependencies for auto-detection of header content update.
#

