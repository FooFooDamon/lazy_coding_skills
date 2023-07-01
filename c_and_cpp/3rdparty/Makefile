OTHER_CFLAGS := -DTEST

C_SRCS := $(shell find ./ -name "*.c" | grep -v "./windows_only")

GOALS := $(addsuffix .elf, $(basename ${C_SRCS}))

all:

${GOALS}: %.elf: %.o

help:
	@echo "Available commands:"
	@echo "  all        - Generate test executables, with or without \"all\"."
	@echo "  check      - Do static checkings."
	@echo "  clean      - Remove all generated files."
	@echo "  *-release  - Generate formal executables for a specific architecture."
	@echo "  *-debug    - Generate debugging executables for a specific architecture."
	@echo 'Run with "V=1" or "VERBOSE=1", and find out supported architectures from ARCH_LIST.'

include ../../makefile/c_and_cpp.mk

