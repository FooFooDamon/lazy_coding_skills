CC := gcc

C_INCLUDES :=
OTHER_CFLAGS := -DTEST

SRCS := $(shell find ./ -name "*.c")
OBJS := $(addsuffix .o, $(basename ${SRCS}))

all: ${OBJS}

EXECS := $(filter-out , $(basename ${SRCS}))

${EXECS}: %: %.o
	${CXX_LINK}

test:
	for i in ${EXECS}; \
	do \
		make $$i; \
	done

check:
	clang --analyze $(SRCS) ${CFLAGS}

clean:
	rm -f ${EXECS} ${OBJS} *.plist

include ../../makefile/basic_rules_for_c_and_cpp.mk

