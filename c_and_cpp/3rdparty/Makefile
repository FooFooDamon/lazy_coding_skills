CC := gcc

C_INCLUDES :=
OTHER_CFLAGS := -DTEST

SRCS := $(shell find ./ -name "*.c" | grep -v "./windows_only")
OBJS := $(addsuffix .o, $(basename ${SRCS}))

all: ${OBJS}

EXECS :=

${EXECS}: %: %.o
	${CXX_LINK}

test:
	for i in ${EXECS}; \
	do \
		make $$i; \
	done

clean:
	rm -f ${EXECS} ${OBJS}

include ../../makefile/basic_rules_for_c_and_cpp.mk
