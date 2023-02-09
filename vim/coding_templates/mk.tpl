#!/usr/bin/env make

#
# TODO: Brief description of this makefile.
#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

all: build

build: init
	@echo "build: TODO ..."

init:
	@echo "init: Unneeded."

install: build
	@echo "install: TODO ..."

uninstall:
	@echo "uninstall: TODO ..."

check:
	@echo "check: Unneeded."

test:
	@echo "test: Unneeded."

dist:
	@echo "dist: Unneeded."

distclean:
	@echo "distclean: Unneeded."

clean:
	@echo "clean: TODO ..."

help:
	@echo "Available commands:"
	@echo "  all       - Build all targets."
	@echo "  build     - Same as above."
	@echo "  init      - Initialize necessary stuff."
	@echo "* examples  - Show examples."
	@echo "* install   - Install all targets."
	@echo "* uninstall - Uninstall all targets."
	@echo "  check     - Perform some checkings, e.g. code static check."
	@echo "  test      - Run customized tests."
	@echo "  dist      - Make a distribution package for product release."
	@echo "  distclean - Remove all generated files and directories."
	@echo "  clean     - Remove most generated files and directories."

examples:
	@echo "make && make install"
	@echo "make clean"
	@echo "make uninstall"
	@echo "make check"

#
# ================
#   CHANGE LOG
# ================
#
# >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
#   01. Create.
#
