#!/usr/bin/make -f

#
# Default extra build targets for STM32 MCUs.
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

flash_as_storage:
	for i in USER_VECT_TAB_ADDRESS VECT_TAB_SRAM; do \
		sed -i "s/\(\/\*\)*[ ]*\(#define[ ]\+$${i}\>\)[ ]*\(\*\/\)*/\/\* \2 \*\//" Core/Src/system_stm32*.c; \
	done

ram_as_storage:
	for i in USER_VECT_TAB_ADDRESS VECT_TAB_SRAM; do \
		sed -i "s/\(\/\*\)*[ ]*\(#define[ ]\+$${i}\>\)[ ]*\(\*\/\)*/\2/" Core/Src/system_stm32*.c; \
	done

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2024-07-18, Man Hung-Coeng <udc577@126.com>:
#   01. Initial release.
#

