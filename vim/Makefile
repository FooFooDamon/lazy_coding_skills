#!/usr/bin/env make

#
# Installation script of this directory.
#
# Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
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

LCS_USER ?= ${USER}
LCS_EMAIL ?= ${USER}@123456.com
VIM_RC := ${HOME}/.vimrc

all: build

build: init
	@echo "build: Unneeded"

init:
	@echo "init: Unneeded."

install: uninstall
	echo "let g:LCS_USER = '${LCS_USER}'" >> ${VIM_RC}
	echo "let g:LCS_EMAIL = '${LCS_EMAIL}'" >> ${VIM_RC}
	echo "source $(shell pwd)/main.vim" >> ${VIM_RC}

uninstall:
	export vimrc_linenum=$$(grep -n "$(shell pwd)/main.vim" ${VIM_RC} | awk -F : '{ print $$1 }'); \
	if [ -n "$${vimrc_linenum}" ]; then \
		sed -i "$${vimrc_linenum}d" ${VIM_RC}; \
	fi
	sed -i "/g:LCS_USER/d" ${VIM_RC}
	sed -i "/g:LCS_EMAIL/d" ${VIM_RC}

check:
	@echo "check: Unneeded."

test:
	@echo "test: Unneeded."

dist:
	@echo "dist: Unneeded."

distclean:
	@echo "distclean: Unneeded."

clean:
	@echo "clean: Unneeded."

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
	@echo "make install LCS_USER='Man Hung-Ceong' LCS_EMAIL='udc577@126.com'"
	@echo "make install LCS_USER='Man Hung-Ceong'"
	@echo "make install LCS_EMAIL='udc577@126.com'"
	@echo "make install"
	@echo "make uninstall"

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-02-10, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

