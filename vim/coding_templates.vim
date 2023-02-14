"
" Providing templates for file creation of many languages.
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

let THIS_DIR = fnamemodify(expand('<sfile>:p'), ':h')
let $BASENAME = fnamemodify(expand('<sfile>:p'), ':t:r')
let PUBLIC_TEMPLATE_DIR = THIS_DIR . "/" . $BASENAME
let PUBLIC_TEMPLATES = split(globpath(PUBLIC_TEMPLATE_DIR, '*.tpl'), '\n')
let PRIVATE_TEMPLATE_DIR = THIS_DIR . "/private/" . $BASENAME
let PRIVATE_TEMPLATES = split(globpath(PRIVATE_TEMPLATE_DIR, '*.tpl'), '\n')
"let LCS_USER = expand(has_key(environ(), 'LCS_USER') ? "$LCS_USER" : "$USER")
let LCS_USER = exists('g:LCS_USER') ? g:LCS_USER : expand("$USER")
"let LCS_EMAIL = has_key(environ(), 'LCS_EMAIL') ? expand("$LCS_EMAIL") : expand("$USER")."@123456.com"
let LCS_EMAIL = exists('g:LCS_EMAIL') ? g:LCS_EMAIL : expand("$USER")."@123456.com"
let CMD_ADJUST_USER = "g/${LCS_USER}/s//".LCS_USER."/g"
let CMD_ADJUST_EMAIL = "g/${LCS_EMAIL}/s//".LCS_EMAIL."/g"
let CMD_ADJUST_YEAR = "g/${YEAR}/s//".strftime('%Y')."/g"
let CMD_ADJUST_DATE = "g/${DATE}/s//".strftime('%Y-%m-%d')."/g"
let CMD_ADJUST_HEADER_LOCK = "g/${HEADER_LOCK}/s//".toupper(fnamemodify(expand('%:r'), ':t'))."/g"
let CMD_ADJUST_SELF_HEADER = "g/${SELF_HEADER}/s//".fnamemodify(expand('%:r'), ':t')."/g"
let EXEC_ADJUST_ALL = "execute CMD_ADJUST_USER | execute CMD_ADJUST_EMAIL"
    \. " | execute CMD_ADJUST_YEAR | execute CMD_ADJUST_DATE"
    \. " | execute CMD_ADJUST_HEADER_LOCK | execute CMD_ADJUST_SELF_HEADER"

for i in PUBLIC_TEMPLATES
    let template_file = fnamemodify(i, ':t')

    if index(PRIVATE_TEMPLATES, PRIVATE_TEMPLATE_DIR . "/" . template_file) >= 0
        " echo "Found in private directory: " . template_file
        continue
    endif

    for j in split(template_file, '\.')
        if j != 'tpl'
            execute "autocmd BufNewFile *." . j . " 0r " . i . " | " . EXEC_ADJUST_ALL
        endif
    endfor
endfor

for i in PRIVATE_TEMPLATES
    for j in split(fnamemodify(i, ':t'), '\.')
        if j != 'tpl'
            execute "autocmd BufNewFile *." . j . " 0r " . i . " | " . EXEC_ADJUST_ALL
        endif
    endfor
endfor

let $MK_TEMPLATE = PRIVATE_TEMPLATE_DIR . "/" . "mk.tpl"
if index(PRIVATE_TEMPLATES, $MK_TEMPLATE) < 0
    let $MK_TEMPLATE = PUBLIC_TEMPLATE_DIR . "/" . "mk.tpl"
endif

" This will generate two times of original contents for *.mk.
"autocmd BufNewFile,FileType make 0r $MK_TEMPLATE | execute EXEC_ADJUST_ALL

autocmd BufNewFile [Mm][Aa][Kk][Ee][Ff][Ii][Ll][Ee] 0r $MK_TEMPLATE | execute EXEC_ADJUST_ALL
"   \ | execute CMD_ADJUST_USER | execute CMD_ADJUST_EMAIL
"   \ | execute CMD_ADJUST_YEAR | execute CMD_ADJUST_DATE

"
" ================
"   CHANGE LOG
" ================
"
" >>> 2023-02-10, Man Hung-Coeng <udc577@126.com>:
"   01. Create.
"
" >>> 2023-02-14, Man Hung-Coeng <udc577@126.com>:
"   01. Fix the mismatching bug of autocmd BufNewFile.
"   02. Fix the substitution error in creation of a C/C++ source or header
"       file when the file path contains slash(es).
"

