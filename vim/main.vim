"
" The leader script that groups other ones together.
"
" Copyright 2023 Man Hung-Coeng <udc577@126.com>
"
" Licensed under the Apache License, Version 2.0 (the "License");
" you may not use this file except in compliance with the License.
" You may obtain a copy of the License at
"
"     http://www.apache.org/licenses/LICENSE-2.0
"
" Unless required by applicable law or agreed to in writing, software
" distributed under the License is distributed on an "AS IS" BASIS,
" WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
" See the License for the specific language governing permissions and
" limitations under the License.
"

let THIS_PATH = expand('<sfile>:p')
let $THIS_DIR = fnamemodify(THIS_PATH, ':h')
let $PRIVATE_SCRIPT = $THIS_DIR . '/private/' . fnamemodify(THIS_PATH, ':t')

source $THIS_DIR/minimum_settings.vim
source $THIS_DIR/coding_templates.vim
if filereadable($PRIVATE_SCRIPT)
    source $PRIVATE_SCRIPT
endif

"
" ================
"   CHANGE LOG
" ================
" >>> 2023-02-10, Man Hung-Coeng <udc577@126.com>:
"   01. Create.
"

