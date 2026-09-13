# SPDX-License-Identifier: Apache-2.0

#
# Revision number based on VCS (version control system).
#
# Copyright (c) 2026 Man Hung-Coeng <udc577@126.com>
# All rights reserved.
#

VCS ?= git
__REV_LEN__ ?= 8

ifeq (${VCS}, git)

    # Method 1: It works, but is a little tedious.
    __REV_DIRTY_FLAG ?= $(shell \
        [ -z "$$(git diff --name-only . 2> /dev/null | head -n 1)" ] \
        && echo "" \
        || echo "-dirty")
    #
    #VCS_VERSION ?= $(shell \
    #    git log --abbrev-commit --abbrev=${__REV_LEN__} --pretty=oneline . 2> /dev/null \
    #    | head -n 1 \
    #    | awk '{ print $$1 }')${__REV_DIRTY_FLAG}
    ## Or:
    VCS_VERSION ?= $(shell git log --abbrev-commit --abbrev=${__REV_LEN__} --pretty=format:%h -n 1 . 2> /dev/null)${__REV_DIRTY_FLAG}

    # Method 2: This looks fine, but fails to get the commit hash while the latest commit is tagged.
    #VCS_VERSION ?= $(shell \
    #    git describe --abbrev=${__REV_LEN__} --dirty --always 2> /dev/null \
    #    | sed 's/.*\([0-9a-z]\{${__REV_LEN__},\}\)\(\(-dirty\).\?\)/\1\2/')

    # Method 3: Almost the perfect method but it's a pity that
	# this command doesn't support subdirectory when --dirty option is specified!
    #VCS_VERSION ?= $(shell git describe --exclude="*" --abbrev=${__REV_LEN__} --dirty --always 2> /dev/null)

else ifeq (${VCS}, svn)

    __REV_DIRTY_FLAG ?= $(shell \
        [ -z "$$(svn status 2> /dev/null | head -n 1)" ] \
        && echo "" \
        || echo "-dirty")

    VCS_VERSION ?= $(shell \
        LANG=en_US.UTF-8 LANGUAGE=en_US.EN \
        svn info 2> /dev/null \
        | grep 'Last Changed Rev' \
        | sed 's/.* \([0-9]\)/\1/')${__REV_DIRTY_FLAG}

else
    VCS_VERSION ?= <none>
endif

ifeq (${VCS_VERSION},)
    __REVISION__ ?= <none>
else
    __REVISION__ ?= ${VCS_VERSION}
endif

ifneq ($(filter n N no NO No 0, ${EVAL_REVISION_ONCE}),)
    undefine EVAL_REVISION_ONCE
endif
ifdef EVAL_REVISION_ONCE
    export __REV_DIRTY_FLAG VCS_VERSION __REVISION__
endif

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2026-09-13, Man Hung-Coeng <udc577@126.com>:
#   01. Initial commit.
#

