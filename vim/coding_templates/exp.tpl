#!/usr/bin/env expect

#
# TODO: Brief description of this script.
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

proc usage {} {
    global argv0
    set script_name [ file tail "$argv0" ]

    puts stdout "$script_name - <Brief description of this script>"
    puts stdout "Usage: $script_name <Usage format of this script>"
    puts stdout "Example 1: $script_name <An example showing how actual arguments should be like>"
    # TODO: Put more examples if necessary.
}

#proc version {} {
#    global argv0
#    set script_name [ file tail "$argv0" ]
#
#    puts stdout "${script_name}: V1.0.0 ${DATE}"
#}

set timeout -1

foreach i $argv {
    if { "$i" == "-h" } {
        usage
        exit 0
    }
    
#    if { "$i" == "-v" } {
#        version
#        exit 0
#    }
}

# TODO: Your own stuff

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
#   01. Create.
#
