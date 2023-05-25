#!/usr/bin/make -f

#
# STM32CubeIDE makefile wrapper.
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

export MODE ?= Release
export IDE_PARENT_DIR ?= /opt/st
export DEBUG_DEVICE ?= stlink
export CPU_SERIES ?= stm32f1x
export FLASH_ADDR_START ?= 0x08000000

all: build

build:
	export GCC_BIN_DIR=$$(find ${IDE_PARENT_DIR}/stm32cubeide*/ -name arm-none-eabi-gcc | grep "bin/" | xargs -I {} dirname {}); \
	[ -z "$${GCC_BIN_DIR}" ] && : || export PATH=$${GCC_BIN_DIR}:$${PATH}; \
	make -C ${MODE} main-build

build_debug:
	make build MODE=Debug

download:
	[ $$(ls ${MODE}/*.bin | wc -l) -eq 1 ] && openocd -f /usr/share/openocd/scripts/interface/${DEBUG_DEVICE}.cfg \
		-f /usr/share/openocd/scripts/target/${CPU_SERIES}.cfg \
		-c init -c "reset halt" -c wait_halt \
		-c "flash write_image erase $$(ls ${MODE}/*.bin) ${FLASH_ADDR_START}" \
		-c reset -c shutdown || :

gen_ycm_conf:
	make clean
	printf "import os\n" > .ycm_extra_conf.py
	printf "\nYCM_CONF_DIR = os.path.abspath(os.path.dirname(__file__))\n" >> .ycm_extra_conf.py
	printf "\nflags = [\n" >> .ycm_extra_conf.py
	printf '    "-Wall"\n' >> .ycm_extra_conf.py
	printf '    , "-std=gnu11"\n' >> .ycm_extra_conf.py
	printf '    , "-x", "c"\n' >> .ycm_extra_conf.py
	make --dry-run -C ${MODE} main-build | grep 'gcc "[-./A-Za-z0-9_]\+\.c"' | head -n 1 | sed 's/ \+/\n/g' | grep "^-I" \
		| sed -e 's/\(-I\)"\?\([^"]\+\)"\?/    , "\1\2"/' -e 's/-I\.\.\//-I\.\//' \
			-e 's/\(.*-I\)\(\..*\)/\1", os.path.join(YCM_CONF_DIR, "\2)/'>> .ycm_extra_conf.py
	make --dry-run -C ${MODE} main-build | grep 'gcc "[-./A-Za-z0-9_]\+\.c"' | head -n 1 | sed 's/ \+/\n/g' | grep "^-D" \
		| sed -e 's/"/\\"/' -e 's/\(.*\)/    , "\1"/' >> .ycm_extra_conf.py
	printf "]\n" >> .ycm_extra_conf.py
	printf '\nSOURCE_EXTENSIONS = [ ".c" ]\n' >> .ycm_extra_conf.py
	printf '\ndef FlagsForFile(filename, **kwargs):\n' >> .ycm_extra_conf.py
	printf '    return { "flags": flags, "do_cache": True }\n' >> .ycm_extra_conf.py
	printf "\n" >> .ycm_extra_conf.py

clean:
	make -C ${MODE} clean

clean_debug:
	make clean MODE=Debug

#
# ================
#   CHANGE LOG
# ================
#
# >>> 2023-05-25, Man Hung-Coeng <udc577@126.com>:
#   01. Create.
#

