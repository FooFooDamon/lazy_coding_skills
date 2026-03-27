#!/usr/bin/env expect

# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#

proc usage {} {
    global argv0
    set script_name [ file tail "$argv0" ]

    puts stdout "\n$script_name - <Brief description of this script>"
    puts stdout "\nUSAGE: $script_name \[OPTIONS ...\]"
    puts stdout "  -h, --help     Show this help message."
    puts stdout "  -v, --version  Show version info."
    # TODO: Put more options if any.
    puts stdout "\nEXAMPLES:"
    puts stdout "  1. $script_name <An example showing how actual arguments should be like>"
    # TODO: Put more examples if necessary.
    puts stdout ""
}

proc version {} {
    global argv0
    set fd [ open $argv0 r ]
    set version "<none>"

    while { [ gets $fd line ] >= 0 } {
        set pound_sign_index [ string first "#" $line ]

        if { $pound_sign_index != 0 } {
            continue
        }

        set triple_bracket_index [ string first ">>>" $line ]

        if { $triple_bracket_index < 1 } {
            continue
        }

        set v_index [ string first "V" $line ]

        if { $v_index < 4 } {
            continue
        }

        set version [ string range $line $v_index [ expr $v_index + 16 ] ]
    }

    close $fd

    puts stdout $version
}

set timeout -1

foreach i $argv {
    if { "$i" == "-h" || "$i" == "--help" } {
        usage
        exit 0
    } elseif { "$i" == "-v" || "$i" == "--version" } {
        version
        exit 0
    } else {
        continue # TODO: Or whatever you want.
    }
}

# TODO: Your own stuff

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
#   01. Initial commit.
#
