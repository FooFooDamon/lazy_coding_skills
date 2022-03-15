#!/usr/bin/env python
#-*-coding: utf-8-*-

#
# Configuration script of YouCompleteMe VIM plugin,
# which defines common rules for C/C++ grammar checking and code completion.
#
# Copyright 2022 Man Hung-Coeng <udc577@126.com>
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

import os

flags = [
    "-Wall"
    , "-std=c++11"
    , "-x", "c++"
]

for inc_dir in os.popen("echo | $(g++ --print-prog-name=cc1plus) -v 2>&1 | grep '/usr/.*include' | grep -v '^ignoring' | sed 's/^[ ]*//g'").read().split("\n"):
    if "" != inc_dir:
        flags.append("-I")
        flags.append(inc_dir)

SOURCE_EXTENSIONS = [ ".c", ".C", ".cc", ".cpp", ".cxx", ".c++" ]

def FlagsForFile(filename, **kwargs):
    return { "flags": flags, "do_cache": True }

if __name__ == "__main__":
    import sys
    print('*** DO NOT run this script directly!', file = sys.stderr)
    print('Import it into the ".ycm_extra_conf.py" file in your project instead.', file = sys.stderr)

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2022-02-03, Man Hung-Coeng:
#   01. Create.
#
# >>> 2022-03-15, Man Hung-Coeng:
#   01. Add description and changelog.
#

