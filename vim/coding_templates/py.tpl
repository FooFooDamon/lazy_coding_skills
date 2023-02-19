#!/usr/bin/env python
#-*-coding: utf-8-*-

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

from argparse import ArgumentParser, Namespace as ArgumentNamespace

def parse_cmdline() -> ArgumentNamespace:

    parser = ArgumentParser(description = "Brief description of this script")

    parser.add_argument("--version", "-v", action = "store_true", default = False,
        help = "Show version info.")

    return parser.parse_args()

def get_version() -> str:

    version = "[none]"

    with open(__file__, "r") as f:
        for line in f.readlines():
            if line.startswith("# >>> V"):
                version = "V" + line.split(",")[0].split("V")[1]

    return version

def main():

    cmdline_args = parse_cmdline()

    if cmdline_args.version:
        print(get_version())
        exit(0)

    # TODO: Add your own stuff.

if __name__ == "__main__":
    main()

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
#   01. Create.
#
