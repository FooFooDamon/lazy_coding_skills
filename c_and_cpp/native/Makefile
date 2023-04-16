CC := gcc
CXX := g++

C_DEFINES := -DTEST -D__ORDER_LITTLE_ENDIAN__=1234 -D__ORDER_BIG_ENDIAN__=4321 \
		-D__BYTE_ORDER__=__ORDER_LITTLE_ENDIAN__ -DCOMMPROTO_BIG_ENDIAN -D'likely(x)=__builtin_expect(!!(x), 1)' \
		-D'unlikely(x)=__builtin_expect(!!(x), 0)'
CXX_DEFINES := -DTEST -D'likely(x)=__builtin_expect(!!(x), 1)' -D'unlikely(x)=__builtin_expect(!!(x), 0)'

C_INCLUDES :=
CXX_INCLUDES :=

OTHER_CFLAGS :=
OTHER_CXXFLAGS := -std=c++11

C_LDFLAGS :=
CXX_LDFLAGS := -lpthread

C_SRCS := $(shell find ./ -name "*.c")
CXX_SRCS := $(shell find ./ -name "*.cpp")
OBJS := $(addsuffix .o, $(basename ${C_SRCS} ${CXX_SRCS}))

all: ${OBJS}

EXECS := $(filter-out , $(basename ${OBJS}))

${EXECS}: %: %.o
	${CXX_LINK}

test-execs:
	for i in ${EXECS}; \
	do \
		make $$i; \
	done

NON_ANSI_C_SRCS = ./logger_on_syslog.c ./signal_handling.c ./sleeps.c

${NON_ANSI_C_SRCS:.c=.o}: %.o: %.c
	${CC} ${CFLAGS} -std=gnu99 -c -o $@ $<

PARALLEL_OPTION ?= -j $(shell grep -c "processor" /proc/cpuinfo)

check:
	cppcheck --quiet --enable=all --language=c --std=c89 ${PARALLEL_OPTION} ${C_DEFINES} \
		${C_INCLUDES} ${C_SRCS}
	cppcheck --quiet --enable=all --language=c++ --std=c++11 ${PARALLEL_OPTION} -D__cplusplus=201103L ${CXX_DEFINES} \
		${CXX_INCLUDES} ${CXX_SRCS}
	clang --analyze ${C_SRCS} ${CFLAGS} -std=gnu89
	clang --analyze ${CXX_SRCS} ${CXXFLAGS}

clean:
	rm -f ${EXECS} ${OBJS} ${D_FILES} *.plist

help:
	@echo "Available commands:"
	@echo "  all        - Generate object files, with or without \"all\"."
	@echo "  test-execs - Generate test executables."
	@echo "  check      - Do static checkings."
	@echo "  clean      - Remove all generated files."

include ../../makefile/__ver__.mk
include ../../makefile/c_and_cpp.mk

