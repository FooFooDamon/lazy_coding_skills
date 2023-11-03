__STRICT__ := y

C_STD := c89

C_DEFINES := -DTEST -D__ORDER_LITTLE_ENDIAN__=1234 -D__ORDER_BIG_ENDIAN__=4321 \
		-D__BYTE_ORDER__=__ORDER_LITTLE_ENDIAN__ -DCOMMPROTO_BIG_ENDIAN -D'likely(x)=__builtin_expect(!!(x), 1)' \
		-D'unlikely(x)=__builtin_expect(!!(x), 0)'
CXX_DEFINES := -DTEST -D'likely(x)=__builtin_expect(!!(x), 1)' -D'unlikely(x)=__builtin_expect(!!(x), 0)'

CXX_LDFLAGS := -lpthread

C_SRCS := $(shell find ./ -name "*.c" | grep -v "chardev_group\.c")
CXX_SRCS := $(shell find ./ -name "*.cpp")

GOALS := $(addsuffix .elf, $(basename ${C_SRCS} ${CXX_SRCS}))

all:

${GOALS}: %.elf: %.o

NON_ANSI_C_SRCS = ./logger_on_syslog.c ./signal_handling.c ./sleeps.c

${NON_ANSI_C_SRCS:.c=.o}: C_STD = gnu99

help:
	@echo "Available commands:"
	@echo "  all        - Generate test executables., with or without \"all\"."
	@echo "  check      - Do static checkings."
	@echo "  clean      - Remove all generated files."
	@echo "  *-release  - Generate formal executables for a specific architecture."
	@echo "  *-debug    - Generate debugging executables for a specific architecture."
	@echo 'Run with "V=1" or "VERBOSE=1", and find out supported architectures from ARCH_LIST.'

include ../../makefile/__ver__.mk
include ../../makefile/c_and_cpp.mk

