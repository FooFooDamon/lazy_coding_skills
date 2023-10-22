#!/usr/bin/env python
#-*-coding: utf-8-*-

#
# Configuration script of YouCompleteMe VIM plugin,
# which defines common rules for C/C++ grammar checking and code completion.
#
# Copyright (c) 2022-2023 Man Hung-Coeng <udc577@126.com>
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

MAX_SRC_FILES = 50000

#
# For the old working mode based on libclang.
#
def FlagsForFile(filename, **kwargs):
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
    cmd_dict = { "directory": dirpath, "file": filename, "arguments": [ "g++" ] }
    cmd_dict["arguments"].extend(flags)
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
        fp.write(json.dumps(cmd_json, indent = 4, ensure_ascii = False))
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
def Settings(**kwargs):
#{#
    CreateCommandsJsonIfNone(kwargs["filename"])
    return { "flags": flags }
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
    print('    # flags.extend([ "-I", YCM_CONF_DIR ]) # More directories and macros if needed.', file = sys.stderr)
#}#

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2022-02-03, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> 2022-03-15, Man Hung-Coeng <udc577@126.com>:
#   01. Add description and changelog.
#
# >>> 2023-10-06, Man Hung-Coeng <udc577@126.com>:
#   01. Improve the usage guide to make it more descriptive.
#
# >>> 2023-10-22, Man Hung-Coeng <udc577@126.com>:
#   01. Support the new working mode based on clangd.
#

