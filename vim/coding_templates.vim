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

let s:THIS_DIR = fnamemodify(expand('<sfile>:p'), ':h')
let s:BASENAME = fnamemodify(expand('<sfile>:p'), ':t:r')
let s:PUBLIC_TEMPLATE_DIR = s:THIS_DIR . "/" . s:BASENAME
let s:PUBLIC_TEMPLATES = split(globpath(s:PUBLIC_TEMPLATE_DIR, '*.tpl'), '\n')
let s:PRIVATE_TEMPLATE_DIR = s:THIS_DIR . "/private/" . s:BASENAME
let s:PRIVATE_TEMPLATES = split(globpath(s:PRIVATE_TEMPLATE_DIR, '*.tpl'), '\n')

function s:load_template(template)
    echo "Load from a temple? [Y/n] "
    let l:confirm = getcharstr()

    if l:confirm == 'n' || l:confirm == 'N'
        return
    endif

    "let l:LCS_USER = expand(has_key(environ(), 'LCS_USER') ? "$LCS_USER" : "$USER")
    let l:LCS_USER = exists('g:LCS_USER') ? g:LCS_USER : expand("$USER")
    "let l:LCS_EMAIL = has_key(environ(), 'LCS_EMAIL') ? expand("$LCS_EMAIL") : expand("$USER")."@123456.com"
    let l:LCS_EMAIL = exists('g:LCS_EMAIL') ? g:LCS_EMAIL : expand("$USER")."@123456.com"
    let l:CMD_ADJUST_USER = "g/${LCS_USER}/s//".l:LCS_USER."/g"
    let l:CMD_ADJUST_EMAIL = "g/${LCS_EMAIL}/s//".l:LCS_EMAIL."/g"
    let l:CMD_ADJUST_YEAR = "g/${YEAR}/s//".strftime('%Y')."/g"
    let l:CMD_ADJUST_DATE = "g/${DATE}/s//".strftime('%Y-%m-%d')."/g"
    let l:CMD_ADJUST_HEADER_LOCK = "g/${HEADER_LOCK}/s//".toupper(fnamemodify(expand('%:r'), ':t'))."/g"
    let l:CMD_ADJUST_SELF_HEADER = "g/${SELF_HEADER}/s//".fnamemodify(expand('%:r'), ':t')."/g"
    let l:CMD_ADJUST_TITLE = "g/${TITLE}/s//".fnamemodify(expand('%:r'), ':t')."/g"
    let l:EXEC_ADJUST_ALL = "execute l:CMD_ADJUST_USER | execute l:CMD_ADJUST_EMAIL"
        \. " | execute l:CMD_ADJUST_YEAR | execute l:CMD_ADJUST_DATE"
        \. " | execute l:CMD_ADJUST_HEADER_LOCK | execute l:CMD_ADJUST_SELF_HEADER"
        \. " | execute l:CMD_ADJUST_TITLE"

    execute "0r " . a:template . " | " . l:EXEC_ADJUST_ALL
endfunction

for s:i in s:PUBLIC_TEMPLATES
    let s:template_file = fnamemodify(s:i, ':t')

    if index(s:PRIVATE_TEMPLATES, s:PRIVATE_TEMPLATE_DIR . "/" . s:template_file) >= 0
        " echo "Found in private directory: " . s:template_file
        continue
    endif

    for s:j in split(s:template_file, '\.')
        if s:j != 'tpl'
            execute "autocmd BufNewFile *." . s:j . " call s:load_template('" . s:i . "')"
        endif
    endfor
endfor

for s:i in s:PRIVATE_TEMPLATES
    for s:j in split(fnamemodify(s:i, ':t'), '\.')
        if s:j != 'tpl'
            execute "autocmd BufNewFile *." . s:j . " call s:load_template('" . s:i . "')"
        endif
    endfor
endfor

let s:MK_TEMPLATE = s:PRIVATE_TEMPLATE_DIR . "/" . "mk.tpl"
if index(s:PRIVATE_TEMPLATES, s:MK_TEMPLATE) < 0
    let s:MK_TEMPLATE = s:PUBLIC_TEMPLATE_DIR . "/" . "mk.tpl"
endif

execute "autocmd BufNewFile [Mm][Aa][Kk][Ee][Ff][Ii][Ll][Ee] call s:load_template('" . s:MK_TEMPLATE . "')"

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
" >>> 2023-04-10, Man Hung-Coeng <udc577@126.com>:
"   01. Add CMD_ADJUST_TITLE.
"
" >>> 2023-04-12, Man Hung-Coeng <udc577@126.com>:
"   01. Add a confirmation before loading a template.
"   02. Turn all global variables and functions into local ones.
"

