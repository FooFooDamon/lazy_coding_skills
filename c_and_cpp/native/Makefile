CC := gcc

C_DEFINES := -DTEST
C_INCLUDES :=
OTHER_CFLAGS :=

SRCS := $(shell find ./ -name "*.c")
OBJS := $(addsuffix .o, $(basename ${SRCS}))

all: ${OBJS}

EXECS := $(filter-out , $(basename ${SRCS}))

${EXECS}: %: %.o
	${C_LINK}

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
	cppcheck --enable=all --language=c --std=c89 --std=posix ${PARALLEL_OPTION} ${C_DEFINES} ${C_INCLUDES} ${SRCS}
	clang --analyze ${SRCS} ${CFLAGS} -std=gnu89

clean:
	rm -f ${EXECS} ${OBJS} *.plist

include ../../makefile/basic_rules_for_c_and_cpp.mk

