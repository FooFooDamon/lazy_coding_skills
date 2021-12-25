CC := gcc

C_DEFINES := -DTEST
C_INCLUDES :=
OTHER_CFLAGS :=

SRCS := $(shell find ./ -name "*.c")
OBJS := $(addsuffix .o, $(basename ${SRCS}))

all: ${OBJS}

EXECS := $(filter-out , $(basename ${SRCS}))

${EXECS}: %: %.o
	${CXX_LINK}

./signal_handling.o: ./signal_handling.c
	${CC} ${CFLAGS} -std=gnu89 -c -o $@ $<

test:
	for i in ${EXECS}; \
	do \
		make $$i; \
	done

PARALLEL_OPTION ?= -j $(shell grep -c "processor" /proc/cpuinfo)

check:
	cppcheck --enable=all --language=c --std=c89 ${PARALLEL_OPTION} ${C_DEFINES} ${C_INCLUDES} .
	clang --analyze ${SRCS} ${CFLAGS}

clean:
	rm -f ${EXECS} ${OBJS} *.plist

include ../../makefile/basic_rules_for_c_and_cpp.mk

