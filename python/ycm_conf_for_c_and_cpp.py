#!/usr/bin/env python
#-*-coding: utf-8-*-

#
# Configuration script of YouCompleteMe VIM plugin,
# which defines common rules for C/C++ grammar checking and code completion.
#
# Copyright 2022-2023 Man Hung-Coeng <udc577@126.com>
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
    print('\nUsage example 1: Make a symbolic link:', file = sys.stderr)
    print('    $ ln -s ' + os.path.abspath(__file__) + ' /path/to/your/directory/.ycm_extra_conf.py', file = sys.stderr)
    print('\nUsage example 2: Create your own .ycm_extra_conf.py '
        + 'and input (at least) the following lines into it:', file = sys.stderr)
    print('    import os, sys', file = sys.stderr)
    print('    YCM_CONF_DIR = os.path.abspath(os.path.dirname(__file__))', file = sys.stderr)
    print('    sys.path.append("' + os.path.abspath(os.path.dirname(__file__)) + '")', file = sys.stderr)
    print('    from ' + os.path.splitext(os.path.basename(__file__))[0] + ' import *', file = sys.stderr)
    print('    # flags.extend([ "-I", YCM_CONF_DIR ]) # More directories and macros if needed.', file = sys.stderr)

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
# >>> 2023-10-06, Man Hung-Coeng:
#   01. Improve the usage guide to make it more descriptive.
#

