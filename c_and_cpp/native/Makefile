CC := gcc
CXX := g++

C_DEFINES := -DTEST -D__ORDER_LITTLE_ENDIAN__=1234 -D__ORDER_BIG_ENDIAN__=4321 \
		-D__BYTE_ORDER__=__ORDER_LITTLE_ENDIAN__ -DCOMMPROTO_BIG_ENDIAN
CXX_DEFINES := -DTEST

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

test:
	for i in ${EXECS}; \
	do \
		make $$i; \
	done

NON_ANSI_C_SRCS = ./signal_handling.c ./sleeps.c

${NON_ANSI_C_SRCS:.c=.o}: %.o: %.c
	${CC} ${CFLAGS} -std=gnu89 -c -o $@ $<

PARALLEL_OPTION ?= -j $(shell grep -c "processor" /proc/cpuinfo)

check:
	cppcheck --quiet --enable=all --language=c --std=c89 ${PARALLEL_OPTION} ${C_DEFINES} \
		${C_INCLUDES} ${C_SRCS}
	cppcheck --quiet --enable=all --language=c++ --std=c++11 ${PARALLEL_OPTION} -D__cplusplus=201103L ${CXX_DEFINES} \
		${CXX_INCLUDES} ${CXX_SRCS}
	clang --analyze ${C_SRCS} ${CFLAGS} -std=gnu89
	clang --analyze ${CXX_SRCS} ${CXXFLAGS} -std=c++11

clean:
	rm -f ${EXECS} ${OBJS} *.plist

include ../../makefile/basic_rules_for_c_and_cpp.mk

