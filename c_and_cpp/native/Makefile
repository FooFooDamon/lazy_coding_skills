__STRICT__ := y
EVAL_VERSION_ONCE := $(if $(filter driver, ${T}), Y, N)

include ${PWD}/../../makefile/__ver__.mk

MODULES := chardev_group devclass_supplements test_inline_klogging

#=======================
# Applications Only
#=======================
ifeq (${T}, app)

C_STD := c89

C_DEFINES := -DTEST -D__ORDER_LITTLE_ENDIAN__=1234 -D__ORDER_BIG_ENDIAN__=4321 \
		-D__BYTE_ORDER__=__ORDER_LITTLE_ENDIAN__ -DCOMMPROTO_BIG_ENDIAN -D'likely(x)=__builtin_expect(!!(x), 1)' \
		-D'unlikely(x)=__builtin_expect(!!(x), 0)'
CXX_DEFINES := -DTEST -D'likely(x)=__builtin_expect(!!(x), 1)' -D'unlikely(x)=__builtin_expect(!!(x), 0)'

CXX_LDFLAGS := -lpthread

NULL :=
SPACE := ${NULL} ${NULL}
C_SRCS := $(shell find ./ -name "*.c" | grep -v "\($(subst ${SPACE},\|,${MODULES})\|\.mod\)\.c")
CXX_SRCS := $(shell find ./ -name "*.cpp")

GOALS := $(addsuffix .elf, $(basename ${C_SRCS} ${CXX_SRCS}))

all:

${GOALS}: %.elf: %.o

NON_ANSI_C_SRCS = ./camera_v4l2.c ./logger_on_syslog.c ./signal_handling.c

${NON_ANSI_C_SRCS:.c=.o}: C_STD = c99

./signal_handling.o: C_DEFINES += -U__STRICT_ANSI__

include ${PWD}/../../makefile/c_and_cpp.mk

#=======================
# Linux Drivers Only
#=======================
else ifeq (${T}, driver)

ccflags-y += -DTEST
ifeq (${DRVNAME},)
$(error DRVNAME not specified)
endif
export APP_NAME ?= $(basename $(shell ls ${DRVNAME}_app.c 2> /dev/null))

include ${PWD}/../../makefile/linux_driver.mk

#=======================
# Both
#=======================
else

all: apps drivers

check: check-apps

clean: clean-apps clean-drivers

# Q is short for "quiet".
Q := $(if $(strip $(filter-out n N no NO No 0, ${V} ${VERBOSE})),,@)

apps:
	${Q}${MAKE} T=app

check-apps:
	${Q}${MAKE} T=app check

clean-apps:
	${Q}${MAKE} T=app clean

drivers:
	${Q}for i in ${MODULES}; \
	do \
		${MAKE} T=driver DRVNAME=$$i; \
	done

clean-drivers:
	${Q}for i in ${MODULES}; \
	do \
		${MAKE} T=driver DRVNAME=$$i clean; \
	done

ARCH_LIST ?= aarch64 arm avr host mips powerpc x86

$(foreach i, ${ARCH_LIST}, ${i}-release): %:
	${Q}${MAKE} ARCH=${@:-release=}

$(foreach i, ${ARCH_LIST}, ${i}-release-apps): %:
	${Q}${MAKE} ARCH=${@:-release-apps=} apps

$(foreach i, ${ARCH_LIST}, ${i}-release-drivers): %:
	${Q}${MAKE} ARCH=${@:-release-drivers=} drivers

$(foreach i, ${ARCH_LIST}, ${i}-debug): %:
	${Q}${MAKE} ARCH=${@:-debug=} NDEBUG=0

$(foreach i, ${ARCH_LIST}, ${i}-debug-apps): %:
	${Q}${MAKE} ARCH=${@:-debug-apps=} NDEBUG=0 apps

$(foreach i, ${ARCH_LIST}, ${i}-debug-drivers): %:
	${Q}${MAKE} ARCH=${@:-debug-drivers=} NDEBUG=0 drivers

$(foreach i, ${ARCH_LIST}, clean-${i}): %:
	${Q}${MAKE} ARCH=${@:clean-%=%} clean

$(foreach i, ${ARCH_LIST}, clean-${i}-apps): %:
	${Q}${MAKE} ARCH=${@:clean-%-apps=%} clean-apps

$(foreach i, ${ARCH_LIST}, clean-${i}-drivers): %:
	${Q}${MAKE} ARCH=${@:clean-%-drivers=%} clean-drivers

help:
	${Q}echo "Available commands:"
	${Q}echo "  all             - Generate test executables. Note that \"all\" is optional."
	${Q}echo "  check           - Do static checkings."
	${Q}echo "  clean           - Remove all generated files."
	${Q}echo "  <arch>-release  - Generate formal executables for a specific architecture."
	${Q}echo "  <arch>-debug    - Generate debugging executables for a specific architecture."
	${Q}echo "  clean-<arch>    - Clean all architecture-specific files."
	${Q}echo "\nThe \"arch\" can be one of:"
	${Q}echo "  "${ARCH_LIST}

endif

