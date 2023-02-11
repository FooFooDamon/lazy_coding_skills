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
#
LAZY_CODING_HOME := $(patsubst %/,%,$(dir $(shell pwd)))
ifneq ("$(wildcard ${HOME}/.bashrc)","")
	BASH_RC := ${HOME}/.bashrc
else ifneq ("$(wildcard ${HOME}/.bash_profile)","")
	BASH_RC := ${HOME}/.bash_profile
else ifneq ("$(wildcard ${HOME}/.profile)","")
	BASH_RC := ${HOME}/.profile
else ifneq ("$(wildcard ${HOME}/.bash_login)","")
	BASH_RC := ${HOME}/.bash_login
else
	$(error Cannot find .bashrc, .bash_profile, .profile or .bash_login in home directory)
endif

all: build

build: init
	@echo "build: Unneeded."

init:
	-chmod +x *.sh

install: build uninstall
	echo 'export LAZY_CODING_HOME=${LAZY_CODING_HOME}' >> "${BASH_RC}"
	echo '. $${LAZY_CODING_HOME}/scripts/__import__.sh' >> "${BASH_RC}"

uninstall:
	sed -i "/LAZY_CODING_HOME/d" "${BASH_RC}"

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
	@echo "make install"
	@echo "make uninstall"

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-02-11, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

