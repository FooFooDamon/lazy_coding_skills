#!/usr/bin/make -f

#
# Different ways of checking out a project over SVN.
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

ifeq ($(strip ${VCS}),svn)

export CHKOUT_PARENT_DIR ?= $(abspath .)
export CHKOUT_ALIAS ?=
export CHKOUT_TAG ?=
export CHKOUT_HASH ?=
export CHKOUT_STEM ?=
export CHKOUT_URL ?=
export CHKOUT_TAIL_PARAMS ?=
export CHKOUT_METHOD ?= partial
export CHKOUT_PARTIAL_ITEMS ?= \
    # Add more items ahead of this line if needed. \
    # Beware that each line should begin with 4 spaces and end with a backslash.

.PHONY: checkout checkout-partial checkout-all checkout-newest \
    checkout-by-tag checkout-by-tag-once \
    checkout-by-hash checkout-by-hash-once

checkout: checkout-${CHKOUT_METHOD}

checkout-partial:
	@echo "*** ${@}: Operation not supported yet!" >&2
	@exit 1

checkout-all:
	@echo "*** ${@}: Operation not supported yet!" >&2
	@exit 1

checkout-newest:
	@echo "*** ${@}: Operation not supported yet!" >&2
	@exit 1

checkout-by-tag:
	@echo "*** ${@}: Operation not supported yet!" >&2
	@exit 1

checkout-by-tag-once:
	@echo "*** ${@}: Operation not supported yet!" >&2
	@exit 1

checkout-by-hash:
	@echo "*** ${@}: Operation not supported yet!" >&2
	@exit 1

checkout-by-hash-once:
	@echo "*** ${@}: Operation not supported yet!" >&2
	@exit 1

endif # ifeq ($(strip ${VCS}),svn)

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-07-02, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

