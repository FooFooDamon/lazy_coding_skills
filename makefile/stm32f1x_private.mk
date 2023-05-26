#!/usr/bin/make -f

#
# Private makefile for MCUs of STM32F1 series.
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

flash_as_storage:
	cp STM32F1*_ON_FLASH.ld $$(ls STM32F1*_FLASH.ld | grep -v ON_FLASH)
	for i in USER_VECT_TAB_ADDRESS VECT_TAB_SRAM; do \
		sed -i "s/\(\/\*\)*[ ]*\(#define[ ]\+$${i}\>\)[ ]*\(\*\/\)*/\/\* \2 \*\//" Core/Src/system_stm32f1xx.c; \
	done

ram_as_storage:
	cp STM32F1*_ON_RAM.ld $$(ls STM32F1*_FLASH.ld | grep -v ON_FLASH)
	for i in USER_VECT_TAB_ADDRESS VECT_TAB_SRAM; do \
		sed -i "s/\(\/\*\)*[ ]*\(#define[ ]\+$${i}\>\)[ ]*\(\*\/\)*/\2/" Core/Src/system_stm32f1xx.c; \
	done

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-05-26, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

