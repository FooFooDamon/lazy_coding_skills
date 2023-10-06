#!/usr/bin/env python
#-*-coding: utf-8-*-

#
# Configuration script of YouCompleteMe VIM plugin,
# which defines common rules for parsing Linux driver code.
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

import os

THIS_DIR = os.path.abspath(os.path.dirname(__file__))
KERNEL_ROOT = os.path.join("/lib/modules", os.popen("uname -r").read().strip(), "build")
ARCH = os.popen("uname -m | sed 's/\\(x86\\)[-_]\\(32\\|64\\)/\\1/g'").read().strip()

flags = [
    "-Wall"
    , "-std=gnu11"
    , "-x", "c"
    #, "-I", THIS_DIR
    , "-I", os.path.join(THIS_DIR, "..", "c_and_cpp", "native")
    , "-I", os.path.join(KERNEL_ROOT, "arch", ARCH, "include")
    , "-I", os.path.join(KERNEL_ROOT, "arch", ARCH, "include", "generated", "uapi")
    , "-I", os.path.join(KERNEL_ROOT, "arch", ARCH, "include", "generated")
    , "-I", os.path.join(KERNEL_ROOT, "include")
    , "-I", os.path.join(KERNEL_ROOT, "arch", ARCH, "include", "uapi")
    , "-I", os.path.join(KERNEL_ROOT, "include", "uapi")
    , "-I", os.path.join(KERNEL_ROOT, "include", "generated", "uapi")
    , "-include", os.path.join(KERNEL_ROOT, "include", "linux", "kconfig.h")
    #, "-include", os.path.join(KERNEL_ROOT, "include", "linux", "compiler_types.h")
    , "-D__KERNEL__"
    #, "-D__ASSEMBLY__"
    , "-DCC_USING_FENTRY"
    , "-DMODULE"
    , '-DKBUILD_MODNAME="<auto_generated_during_compilation>"'
]

SOURCE_EXTENSIONS = [ ".c" ]

def FlagsForFile(filename, **kwargs):
    return { "flags": flags, "do_cache": True }

if __name__ == "__main__":
    import sys
    print('*** DO NOT run this script directly!', file = sys.stderr)
    print('\nUsage example 1: Make a symbolic link:', file = sys.stderr)
    print('    $ ln -s ' + os.path.abspath(__file__) + ' /path/to/your/.ycm_extra_conf.py', file = sys.stderr)
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
# >>> 2023-10-05, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2023-10-06, Man Hung-Coeng <udc577@126.com>:
#   01. Add KBUILD_MODNAME macro option.
#   02. Improve the usage guide.
#

