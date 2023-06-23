C_INCLUDES :=
OTHER_CFLAGS := -DTEST

SRCS := $(shell find ./ -name "*.c" | grep -v "./windows_only")
OBJS := $(addsuffix .o, $(basename ${SRCS}))

all: ${OBJS}

EXECS := $(addsuffix .elf, $(basename ${OBJS}))

${EXECS}: %.elf: %.o
	${CXX_LINK}

test-execs: ${EXECS}

clean:
	rm -f ${EXECS} ${OBJS} ${D_FILES}

help:
	@echo "Available commands:"
	@echo "  all        - Generate object files, with or without \"all\"."
	@echo "  test-execs - Generate test executables."
	@echo "  clean      - Remove all generated files."

include ../../makefile/c_and_cpp.mk

