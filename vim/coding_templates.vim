"
" Providing templates for file creation of many languages.
"
" Copyright 2023-2024 Man Hung-Coeng <udc577@126.com>
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

function s:wait(hint = "Press any key to continue.")
    if len(a:hint) > 0
        echohl MoreMsg | echo a:hint | echohl None
    endif
    call getchar()
endfunction

function s:load_template(template)
    echo "Load from a temple? [Y/n] "
    "let l:confirm = getcharstr()
    let l:confirm = nr2char(getchar())

    if l:confirm == 'n' || l:confirm == 'N'
        return
    endif

    let l:template_file = a:template
    let l:list_dir = l:template_file . ".list"

    if isdirectory(l:list_dir)
        let l:MAX_ITEMS_OF_ALL = 128
        let l:template_list = filter(readfile(l:template_file), "v:val !~ '^$'")[0:(l:MAX_ITEMS_OF_ALL - 1)]

        if 0 == len(readdir(l:list_dir))
            echohl ErrorMsg | echo "*** No template files in [" . l:list_dir . "] directory!" | echohl None
            return
        else
            let l:template_file = ""
        endif

        let l:MAX_ITEMS_PER_PAGE = 9
        let l:START_PAGE = 1
        let l:END_PAGE = (len(l:template_list) / l:MAX_ITEMS_PER_PAGE)
            \+ !!(len(l:template_list) % l:MAX_ITEMS_PER_PAGE)
        let l:pagenum = 1

        while 0 == len(l:template_file)
            let l:start_index = l:MAX_ITEMS_PER_PAGE * (l:pagenum - 1)
            let l:end_index = l:start_index + l:MAX_ITEMS_PER_PAGE - 1
            let l:cur_page_items = l:template_list[(l:start_index):(l:end_index)]
            let l:hint = "Found multiple template files [" . l:pagenum . "/" . l:END_PAGE . "]:"

            for l:i in range(0, len(l:cur_page_items) - 1)
                let l:hint = l:hint . "\n  " . (l:i + 1) . ". " . l:cur_page_items[(l:i)]
            endfor
            let l:hint = l:hint . "\nInput a number ranging from 1 to " . len(l:cur_page_items) . " to select a template."
            let l:hint = l:hint . "\nOr press \"j\" or Space key to go to next page (if any)."
            let l:hint = l:hint . "\nOr press \"k\" key to go to previous page (if any)."
            let l:hint = l:hint . "\nOr press \"q\" key to quit."
            let l:hint = l:hint . "\nYour choice? [1] "

            redraw!
            echo l:hint

            let l:choice = nr2char(getchar())

            if nr2char(13) == l:choice " Enter
                let l:template_file = l:list_dir . "/" . l:cur_page_items[0]
            elseif 'q' == l:choice || 'Q' == l:choice
                quit
            elseif 'j' == l:choice || 'J' == l:choice || nr2char(32) == l:choice
                if l:pagenum < l:END_PAGE
                    let l:pagenum += 1
                endif
            elseif 'k' == l:choice || 'K' == l:choice
                if l:pagenum > l:START_PAGE
                    let l:pagenum -= 1
                endif
            elseif l:choice >= '1' && l:choice <= string(l:MAX_ITEMS_PER_PAGE)
                let l:template_file = l:list_dir . "/" . l:cur_page_items[(l:choice - 1)]
            else
                echoerr "*** Invalid choice! See the hint above for help."
                call s:wait()
            endif
        endwhile " while 0 == len(l:template_file)
    endif " if isdirectory(l:list_dir)

    if !filereadable(l:template_file)
        echohl ErrorMsg | echo "*** File does not exist or is not allowed to read: " . l:template_file | echohl None
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

    execute "0r " . l:template_file . " | " . l:EXEC_ADJUST_ALL
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
" >>> 2023-05-16, Man Hung-Coeng <udc577@126.com>:
"   01. Replace getcharstr() with nr2char(getchar()) in load_template()
"       for backward compatibility.
"
" >>> 2024-06-14, Man Hung-Coeng <udc577@126.com>:
"   01. Support selection from multiple alternative templates.
"

