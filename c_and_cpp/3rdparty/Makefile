OTHER_CFLAGS := -DTEST

C_SRCS := $(shell find ./ -name "*.c" | grep -v "./windows_only")

EXECS := $(addsuffix .elf, $(basename ${C_SRCS}))

all: ${EXECS}

${EXECS}: %.elf: %.o
	${CXX_LINK}

help:
	@echo "Available commands:"
	@echo "  all        - Generate test executables, with or without \"all\"."
	@echo "  check      - Do static checkings."
	@echo "  clean      - Remove all generated files."

include ../../makefile/c_and_cpp.mk

