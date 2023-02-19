CC := gcc

C_INCLUDES :=
OTHER_CFLAGS := -DTEST

SRCS := $(shell find ./ -name "*.c" | grep -v "./windows_only")
OBJS := $(addsuffix .o, $(basename ${SRCS}))

all: ${OBJS}

EXECS :=

${EXECS}: %: %.o
	${CXX_LINK}

test-execs:
	for i in ${EXECS}; \
	do \
		make $$i; \
	done

clean:
	rm -f ${EXECS} ${OBJS}

help:
	@echo "Available commands:"
	@echo "  all        - Generate object files, with or without \"all\"."
	@echo "  test-execs - Generate test executables."
	@echo "  clean      - Remove all generated files."

include ../../makefile/basic_rules_for_c_and_cpp.mk

