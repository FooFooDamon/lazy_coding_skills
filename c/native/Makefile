CC := gcc

C_INCLUDES :=
OTHER_CFLAGS := -DTEST

SRCS := $(shell find ./ -name "*.c")
OBJS := $(addsuffix .o, $(basename ${SRCS}))

all: ${OBJS}

TARGETS := $(filter-out , $(basename ${SRCS}))

${TARGETS}: %: %.o
	${CXX_LINK}

test:
	for i in ${TARGETS}; \
	do \
		make $$i; \
	done

clean:
	rm -f ${TARGETS} ${OBJS}

include ../../makefile/basic_rules_for_c_and_cpp.mk

