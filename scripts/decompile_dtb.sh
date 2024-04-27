#!/bin/bash

#
# Copyright (c) 2024 Man Hung-Coeng <udc577@126.com>
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

DEFAULT_CONF_FILE="./fdt_phandle_rules.conf"

usage()
{
    printf "\n$(basename $0) - Decompile DTB (Device Tree BLOB) file\n"
    printf "\nUSAGE: $(basename $0) [OPTIONS ...] /path/to/xxx.dtb[o]\n"
    printf "  -h, --help        Show this help message.\n"
    printf "  -v, --version     Show version info.\n"
    printf "  -c, --config /path/to/config/file\n"
    printf "                    Specify the configuration file of phandle rules.\n"
    printf "                    Default to ${DEFAULT_CONF_FILE} if not specified.\n"
    printf "  -g, --generate    Generate a configuration file for further editing.\n"
    printf "  -q, --quiet       Print as little info as possible during execution.\n"
    printf "\nEXAMPLES:\n"
    printf "  1. $(basename $0) -g\n"
    printf "  2. $(basename $0) -g -c ~/my_phandle_rules.conf\n"
    printf "  3. $(basename $0) ./xxx.dtb\n"
    printf "  4. $(basename $0) -c ~/my_phandle_rules.conf ./xxx.dtb\n"
    printf "  5. $(basename $0) ./yyy.dtbo\n"
    printf "  6. $(basename $0) /boot/zzz.dtb -q\n"
    printf "\n"
}

version()
{
    grep "^# >>> V[0-9.]\+[ ]*|" "$0" | tail -n 1 | sed 's/.*\(V[0-9.]\+[ ]*|[0-9-]\+\),.*/\1/'
}

printW()
{
    printf "\e[0;33m$*\e[0m\n" >&2
}

printE()
{
    printf "\e[0;31m$*\e[0m\n" >&2
}

eexit()
{
    [ $# -gt 0 ] && printE "$*"
    exit 1
}

DATETIME_CMD="date +%Y-%m-%d_%H:%M:%S.%N"

handle_sigINT()
{
    printW "$(${DATETIME_CMD}): $(basename $0): Script will exit soon."
    exit 1
}

handle_sigQUIT()
{
    printW "$(${DATETIME_CMD}): $(basename $0): Script will exit soon."
    exit 1
}

SIGNAL_ITEMS=(INT QUIT)
for i in ${SIGNAL_ITEMS[@]}
do
    trap "handle_sig${i}" ${i}
done

verbose=1
is_gen_mode=0
conf_flag=-1
conf_file="${DEFAULT_CONF_FILE}"
dtb_file=""

for i in "$@"
do
    if [ "${i}" = "-h" ] || [ "${i}" = "--help" ]; then
        usage
        exit 0
    elif [ "${i}" = "-v" ] || [ "${i}" = "--version" ]; then
        version
        exit 0
    elif [ "${i}" = "-q" ] || [ "${i}" = "--quiet" ]; then
        verbose=0
    elif [ "${i}" = "-c" ] || [ "${i}" = "--config" ]; then
        [ ${conf_flag} -ge 0 ] || conf_flag=$((conf_flag + 1))
    elif [ "${i}" = "-g" ] || [ "${i}" = "--generate" ]; then
        is_gen_mode=1
    else
        if [ "${i##*.}" = "conf" -a ${conf_flag} -eq 0 ]; then
            conf_file="${i}"
            conf_flag=$((conf_flag + 1))
        else
            dtb_file="${i}"
        fi
    fi
done

display_config_template()
{
    printf "#\n"
    printf "# Example 1: phy-handle = <&ethernet_phy1>;\n"
    printf "# Example 2: reset-gpio = <&gpio6 7 GPIO_ACTIVE_HIGH>;\n"
    printf "#\n"
    printf "[single]\n"
    printf "\tbacklight\n"
    printf "\tinterrupt-parent\n"
    printf "\tphy-handle\n"
    printf "\tremote-endpoint\n"
    printf "\treset-gpio\n"
    printf "\tvbus-supply\n"
    printf "\tvin-supply\n"
    printf "\tvref-supply\n"
    printf "[/single]\n\n"

    printf "#\n"
    printf "# Example 1: nvmem-cells = <&adc_big_scale>, <&adc_small_scale>;\n"
    printf "# Example 2: operating-points-v2 = <&powerdomain0_opp_table>,\n"
    printf "#                                  <&powerdomain1_opp_table>,\n"
    printf "#                                  <&powerdomain2_opp_table>;\n"
    printf "#\n"
    printf "[multiple]\n"
    printf "\tnvmem-cells\n"
    printf "\toperating-points-v2\n"
    printf "[/multiple]\n\n"

    printf "#\n"
    printf "# in-groups-<items per group>-<phandle1 position>-...-<phandleN position>\n"
    printf "#\n"
    printf "# NOTE: Starting value of a position IS 1, not 0.\n"
    printf "#\n"
    printf "# Example 1 (in-groups-2-1): io-channels = <&adc 0>, <&adc 1>, <&adc 2>;\n"
    printf "# Example 2 (in-groups-4-2): msi-map = <0x0000 &msi 0x8000 0x8000>, <0x8000 &msi 0x0000 0x8000>;\n"
    printf "#\n\n"

    printf "[in-groups-2-1]\n"
    printf "\tio-channels\n"
    printf "[/in-groups-2-1]\n\n"
    printf "[in-groups-4-1]\n"
    printf "\tgpio-ranges\n"
    printf "[/in-groups-4-1]\n\n"
    printf "[in-groups-4-2]\n"
    printf "\tmsi-map\n"
    printf "[/in-groups-4-2]\n\n"

    printf "# You can define more [in-groups-*] sections according to your needs.\n\n"
}

[ ${conf_flag} -ne 0 ] || eexit "*** Missing argument of -c"
if [ ${is_gen_mode} -eq 1 ]; then
    [ ! -e "${conf_file}" ] || eexit "*** Configuration already exists: ${conf_file}"
    display_config_template > "${conf_file}" || exit $?
    [ ${verbose} -eq 0 ] || printf "Generated configuration: ${conf_file}\nNow you can add/delete items to/from it.\n"
    exit 0
else
    [ -e "${conf_file}" ] || eexit "*** Configuration not found: ${conf_file}\nRun with \"-g\" to generate one."
    [ ${verbose} -eq 0 ] || printf "\nCONF: ${conf_file}\n"
fi

[ -n "${dtb_file}" ] || eexit "*** DTB file not specified"
[ -e "${dtb_file}" ] || eexit "*** File does not exist: ${dtb_file}"
[ "${dtb_file##*.}" = "dtb" -o "${dtb_file##*.}" = "dtbo" ] || eexit "*** A DTB filename must end with .dtb or .dtbo"
[ ${verbose} -eq 0 ] || printf "\nDTB: ${dtb_file}\n"
coarse_file="${dtb_file%.*}.decompiled.dts"
fixup_file="${dtb_file%.*}.fixup.dts"
tmp_file=/tmp/"$(basename "${fixup_file}")" # Use tmpfs (memory filesystem) to reduce disk I/O operations.

[ -n "${DTC}" ] || DTC=$(which dtc)
[ -n "${DTC}" ] || eexit "*** Missing dtc program!\nInstall it, or specify it through environment variable DTC."
[ -e "${DTC}" ] || eexit "*** File does not exist: ${DTC}"
[ ${verbose} -eq 0 ] || printf "\nDTC: ${DTC}\n"

shopt -s expand_aliases

#[ ${verbose} -eq 0 ] && TIME_STAT="" || TIME_STAT=time
[ ${verbose} -eq 0 ] && alias TIME_STAT="" || alias TIME_STAT="time"

declare -A PHANDLE_CONF_MAP
declare -A GROUP_SIZE_MAP
declare -A GROUP_POSITIONS_MAP

[ ${verbose} -eq 0 ] || printf "\n>>> Started mapping phandle rules based on CONF.\n"
TIME_STAT for i in single multiple $(grep "\[in-groups-[0-9-]\+\]" "${conf_file}" | sed "s/\[\(.\+\)\]/\1/")
do
    if [ "${i:0:9}" = "in-groups" ]; then
        tmp_positions=($(echo ${i#in-groups-*} | sed 's/-/ /g'))
        tmp_size=${tmp_positions[0]}
        unset tmp_positions[0]
    fi

    for j in $(sed -n "/\[${i}\]/,/\[\/${i}\]/p" "${conf_file}" | grep -v "\[\/*${i}\]")
    do
        PHANDLE_CONF_MAP["${j}"]="${i}"
        [ "${i:0:9}" = "in-groups" ] || continue
        GROUP_SIZE_MAP["${j}"]="${tmp_size}"
        GROUP_POSITIONS_MAP["${j}"]="${tmp_positions[*]}"
    done
done
[ ${verbose} -eq 0 ] || printf "\n<<< Finished mapping phandle rules.\n"

[ ${verbose} -eq 0 ] || printf "\n>>> Started decompiling DTB file.\n\n"
[ ${verbose} -eq 0 ] || set -x
TIME_STAT ${DTC} --sort --in-format=dtb --out-format=dts --out "${coarse_file}" "${dtb_file}"
[ ${verbose} -eq 0 ] || set +x
[ ${verbose} -eq 0 ] || printf "\n<<< Finished decompiling DTB file. Result: ${coarse_file}\n"

FIELD_NAME_CHARSET="[-_@+,.0-9a-zA-Z]"
PHANDLE_ASSIGNMENT_REGEX="^[[:blank:]]*\(linux,\)*phandle = <"
phandle_stack=()
declare -A phandle_map

[ ${verbose} -eq 0 ] || printf "\n>>> Started mapping phandle names and values (may take a while).\n\nBe patient ...\n"
TIME_STAT while read i
do
    if [ $(echo "${i}" | grep -c "${PHANDLE_ASSIGNMENT_REGEX}") -gt 0 ]; then
        phandle=$(echo "${i}" | sed 's/.*<\(0x[0-9a-z]\+\)>.*/\1/')
        index=$((${#phandle_stack[*]} - 1))

        phandle_map["${phandle}"]="${phandle_stack[${index}]}"

        unset phandle_stack[${index}]
    else
        #phandle_stack[${#phandle_stack[*]}]=$(echo "${i}" | awk '{ print $1 }')
        phandle_stack+=($(echo "${i}" | awk '{ print $1 }'))
    fi
done <<< $(grep "${PHANDLE_ASSIGNMENT_REGEX}\|^[[:blank:]]*${FIELD_NAME_CHARSET}\+ {$" "${coarse_file}")
# Or:
#shopt -s lastpipe
#grep "..." "${coarse_file}" | while read i
#do
#    ...
#done
[ ${verbose} -eq 0 ] || printf "\n<<< Finished mapping phandle names and values.\n"

sed "/${PHANDLE_ASSIGNMENT_REGEX}/d" "${coarse_file}" > "${tmp_file}"

sed -i "\$s/\(};\)/\n\t__phandles__ { };\n\n\t__possible_targets_to_fix_up__ { };\n\1/" "${tmp_file}"

printf "\n&__phandles__ {\n" >> "${tmp_file}"
for i in ${!phandle_map[*]}
do
    printf "\tphandle_%d_${i} = \"${phandle_map[${i}]}\";\n" ${i}
done | sort -V  >> "${tmp_file}"
echo "};" >> "${tmp_file}"

printf "\n&__possible_targets_to_fix_up__ {\n" >> "${tmp_file}"
grep "^[[:blank:]]*${FIELD_NAME_CHARSET}\+ = <" "${coarse_file}" | grep -v "${PHANDLE_ASSIGNMENT_REGEX}" \
    | awk '{ print "\t"$1";" }' | sort -V | uniq >> "${tmp_file}"
echo "};" >> "${tmp_file}"

[ ${verbose} -eq 0 ] || printf "\n>>> Started restoring phandle references (may take a while).\nBe patient ...\n"
TIME_STAT grep -n "^[[:blank:]]*${FIELD_NAME_CHARSET}\+ = <" "${tmp_file}" | while read i
do
    line_target_phandle=($(echo "${i}" | awk '{ print $1, $2, $4 }'))
    phandle_value=${line_target_phandle[2]:1} # NOTE: May contain a trailing ">" character.
    target_name=${line_target_phandle[1]}
    target_type=${PHANDLE_CONF_MAP["${target_name}"]}

    [ -n "${target_type}" ] && linenum=${line_target_phandle%%:*} || continue

    if [ "${target_type}" = "single" ]; then
        phandle_value=${phandle_value%%>*} # NOTE: Remove the trailing ">" character if any.
        phandle_name=${phandle_map[${phandle_value}]}

        if [ -n "${phandle_name}" ]; then
            sed -i "${linenum}s/^\([ \t]*${target_name} = <\)${phandle_value}\([^>]*>;\)/\1\&${phandle_name}\2/" "${tmp_file}"
        else
            printW "*** ${i} /* Invalid phandle：${phandle_value} */"
        fi
    else
        [ "${target_type}" = "multiple" ] && group_size=0 || group_size=${GROUP_SIZE_MAP["${target_name}"]}
        [ ${group_size} -eq 0 ] && position_array=() || position_array=(${GROUP_POSITIONS_MAP["${target_name}"]})
        phandle_array=()
        index=0

        for j in $(echo "${i}" | sed "s/^${linenum}:[ \t]*${target_name} = <\([^>]\+\)>;/\1/")
        do
            if [ ${group_size} -eq 0 ]; then
                phandle_name=${phandle_map[${j}]}
            else
                [ $(echo "${position_array[*]}" | grep -c "\<$((${index} % ${group_size} + 1))\>") -eq 0 ] \
                    && phandle_name="" || phandle_name=${phandle_map[${j}]}
                index=$((index + 1))
            fi

            [ -z "${phandle_name}" ] && phandle_array+=("${j}") || phandle_array+=("\&${phandle_name}")
        done

        sed -i "${linenum}s/^\([ \t]*${target_name} = <\)[^>]\+\(>;\)/\1${phandle_array[*]}\2/g" "${tmp_file}"
    fi
done
[ ${verbose} -eq 0 ] || printf "\n<<< Finished restoring phandle references.\n"

mv "${tmp_file}" "${fixup_file}"
[ ${verbose} -eq 0 ] || printf "\nResult: ${fixup_file}\n"

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2024-04-18, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#
# >>> V1.0.1|2024-04-27, Man Hung-Coeng <udc577@126.com>:
#   01. Fix a typo: DTC=$(which {dtcc -> dtc}).
#

