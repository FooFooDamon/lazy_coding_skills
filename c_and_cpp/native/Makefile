CC := gcc

C_DEFINES := -DTEST -DCOMMPROTO_BIG_ENDIAN
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
	cppcheck --enable=all --language=c --std=c89 --std=posix ${PARALLEL_OPTION} ${C_DEFINES} \
		-D__ORDER_LITTLE_ENDIAN__=1234 -D__ORDER_BIG_ENDIAN__=4321 -D__BYTE_ORDER__=__ORDER_LITTLE_ENDIAN__ \
		${C_INCLUDES} ${SRCS}
	clang --analyze ${SRCS} ${CFLAGS} -std=gnu89

clean:
	rm -f ${EXECS} ${OBJS} *.plist

include ../../makefile/basic_rules_for_c_and_cpp.mk

