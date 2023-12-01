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
KERNEL_ROOT = os.environ.get("KERNEL_ROOT", os.path.join("/lib/modules", os.popen("uname -r").read().strip(), "build"))
ARCH = os.environ.get("ARCH", os.popen("uname -m | sed 's/\\(x86\\)[-_]\\(32\\|64\\)/\\1/g'").read().strip())

flags = [
    "-Wall"
    , "-std=gnu11"
    , "-x", "c"
    , "-nostdinc"
    , "-I" + THIS_DIR
    , "-I" + os.path.join(THIS_DIR, "..", "c_and_cpp", "native")
    , "-I" + os.path.join(KERNEL_ROOT, "arch", ARCH, "include")
    , "-I" + os.path.join(KERNEL_ROOT, "arch", ARCH, "include", "generated", "uapi")
    , "-I" + os.path.join(KERNEL_ROOT, "arch", ARCH, "include", "generated")
    , "-I" + os.path.join(KERNEL_ROOT, "include")
    , "-I" + os.path.join(KERNEL_ROOT, "arch", ARCH, "include", "uapi")
    , "-I" + os.path.join(KERNEL_ROOT, "include", "uapi")
    , "-I" + os.path.join(KERNEL_ROOT, "include", "generated", "uapi")
    , "-include", os.path.join(KERNEL_ROOT, "include", "linux", "kconfig.h")
    #, "-include", os.path.join(KERNEL_ROOT, "include", "linux", "compiler_types.h")
    , "-D__KERNEL__"
    #, "-D__ASSEMBLY__"
    , "-DCC_USING_FENTRY"
    , "-DMODULE"
    , '-DKBUILD_MODNAME="<auto_generated_during_compilation>"'
]

SOURCE_EXTENSIONS = [ ".c" ]

MAX_SRC_FILES = 50000

#
# For the old working mode based on libclang.
#
def FlagsForFile(filename: str, **kwargs):
#{#
    return { "flags": flags, "do_cache": True }
#}#

def FindNearestTargetFromBottomUp(start_dir: str, filename: str):
#{#
    dest_dir = start_dir

    while '/' != dest_dir and '' != dest_dir and not os.path.exists(os.path.join(dest_dir, filename)):
        dest_dir = os.path.dirname(dest_dir)

    return None if ('/' == dest_dir or '' == dest_dir) else os.path.join(dest_dir, filename)
#}#

def MakeCommandDictItem(dirpath: str, filename: str):
#{#
    cmd_dict = { "directory": dirpath, "file": filename, "arguments": [ os.environ.get("CC", "gcc") ] }
    cmd_dict["arguments"].extend(FlagsForFile(filename)["flags"])
    cmd_dict["arguments"].extend([ "-c", "-o", os.path.splitext(cmd_dict["file"])[0] + ".o" ])
    cmd_dict["arguments"].append(cmd_dict["file"])

    return cmd_dict
#}#

def CreateCommandsJsonIfNone(current_file: str):
#{#
    if os.path.splitext(current_file)[1] not in SOURCE_EXTENSIONS:
        return

    this_dir, this_file = os.path.split(current_file)
    ycm_file = FindNearestTargetFromBottomUp(this_dir, ".ycm_extra_conf.py")

    if None == ycm_file or None != FindNearestTargetFromBottomUp(this_dir, "compile_commands.json"):
        return

    import json

    src_count = 0
    cmd_json = []
    json_dir = os.path.dirname(ycm_file)
    json_file = os.path.join(json_dir, "compile_commands.json")

    for root, dirs, files in os.walk(json_dir, followlinks = True):
    #{#
        for f in files:
        #{#
            if os.path.splitext(f)[1] not in SOURCE_EXTENSIONS:
                continue

            cmd_json.append(MakeCommandDictItem(root, f))

            src_count += 1
            if src_count >= MAX_SRC_FILES:
                break
        #}#

        if src_count >= MAX_SRC_FILES:
            break
    #}# for os.walk(json_dir)

    with open(json_file, "w") as fp:
        fp.write(json.dumps(cmd_json, indent = 2, ensure_ascii = False))
        print("Created: " + json_file)

    if src_count >= MAX_SRC_FILES:
    #{#
        raise RuntimeWarning(
            "*** " + json_file + ": Too many source files, only the first " + str(src_count) + " were chosen!!!"
        )
    #}#
#}# def CreateCommandsJsonIfNone()

#
# For the new working mode based on clangd.
#
# This mode requires that clangd be installed properly
# and compile_commands.json be generated correctly!
#
def Settings(**kwargs):
#{#
    # The user should generate compile_commands.json using another tool
    # rather than depend on auto-creation by the function below.
    CreateCommandsJsonIfNone(kwargs["filename"])

    return FlagsForFile(kwargs["filename"])
#}#

if __name__ == "__main__":
#{#
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
    print('    # flags.extend([ "-I" + YCM_CONF_DIR ]) # More directories and macros if needed.', file = sys.stderr)
#}#

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
# >>> 2023-10-22, Man Hung-Coeng <udc577@126.com>:
#   01. Support the new working mode based on clangd.
#
# >>> 2023-11-01, Man Hung-Coeng <udc577@126.com>:
#   01. Add directory of this script to the list of directories
#       to be searched for header files.
#   02. Add a new way for variable KERNEL_ROOT and ARCH
#       to get their values from environment variables
#       before taking default values.
#
# >>> 2023-11-04, Man Hung-Coeng <udc577@126.com>:
#   01. Use FlagsForFile() as the only entry to get compilation flags.
#
# >>> 2023-12-01, Man Hung-Coeng <udc577@126.com>:
#   01. Support the customization of compiler through environment variable CC.
#   02. Merge each pair of "-I" and its value to one element of flags list,
#       and reduce the indent level of JSON serialization from 4 to 2,
#       so that the generated compile_commands.json is more compact and smaller.
#

