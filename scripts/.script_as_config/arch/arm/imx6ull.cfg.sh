#!/bin/bash

#
# Configuration file of i.MX6ULL for IOMUX checking scripts.
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

. $(dirname ${BASH_SOURCE})/imx6ul.cfg.sh

HEADER_FILES=(
    "${KERNEL_ROOT}/arch/arm/boot/dts/imx6ul-pinfunc.h"
    "${KERNEL_ROOT}/arch/arm/boot/dts/imx6ull-pinfunc-snvs.h"
    "${KERNEL_ROOT}/arch/arm/boot/dts/imx6ull-pinfunc.h"
)
MUX_PREFIX="MX6UL[L]\?_PAD_"

#
# ================
#   CHANGE LOG
# ================
#
# >>> V1.0.0|2023-04-22, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

