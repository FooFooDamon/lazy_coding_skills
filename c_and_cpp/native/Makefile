__STRICT__ := y

C_STD := c89

C_DEFINES := -DTEST -D__ORDER_LITTLE_ENDIAN__=1234 -D__ORDER_BIG_ENDIAN__=4321 \
		-D__BYTE_ORDER__=__ORDER_LITTLE_ENDIAN__ -DCOMMPROTO_BIG_ENDIAN -D'likely(x)=__builtin_expect(!!(x), 1)' \
		-D'unlikely(x)=__builtin_expect(!!(x), 0)'
CXX_DEFINES := -DTEST -D'likely(x)=__builtin_expect(!!(x), 1)' -D'unlikely(x)=__builtin_expect(!!(x), 0)'

CXX_LDFLAGS := -lpthread

C_SRCS := $(shell find ./ -name "*.c")
CXX_SRCS := $(shell find ./ -name "*.cpp")

EXECS := $(filter-out , $(addsuffix .elf, $(basename ${C_SRCS} ${CXX_SRCS})))

.PHONY: all
all: ${EXECS}

${EXECS}: %.elf: %.o
	${CXX_LINK}

NON_ANSI_C_SRCS = ./logger_on_syslog.c ./signal_handling.c ./sleeps.c

${NON_ANSI_C_SRCS:.c=.o}: %.o: %.c
	${CC} ${CFLAGS} --std=gnu99 -c -o $@ $<

help:
	@echo "Available commands:"
	@echo "  all        - Generate test executables., with or without \"all\"."
	@echo "  check      - Do static checkings."
	@echo "  clean      - Remove all generated files."

include ../../makefile/__ver__.mk
include ../../makefile/c_and_cpp.mk

