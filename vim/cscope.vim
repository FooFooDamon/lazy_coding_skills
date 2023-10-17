"
" Settings of cscope plugin.
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

function CscopeConfigIsEnabled()
    return !exists('s:CSCOPE_CONF_DISABLED')
        \ || '' == s:CSCOPE_CONF_DISABLED
        \ || 0 == match(trim(string(s:CSCOPE_CONF_DISABLED[0]), "'"), '[0nN]')
endfunction

function EnableCscopeConfig()
    call s:save_old_key_mappings()
    call s:apply_new_key_mappings()

    let s:CSCOPE_CONF_DISABLED = 0
endfunction

function DisableCscopeConfig()
    let s:CSCOPE_CONF_DISABLED = 1

    call s:restore_old_key_mappings()
endfunction

function s:go_to_definition_via_cscope(target)
    cclose
    execute 'cscope find g ' . a:target
endfunction

function GoToDefinitionViaCscopeOrCtags()
    if !CscopeConfigIsEnabled()
        return
    endif

    let l:ctags_conf_usable = (exists('*CtagsConfigIsEnabled') && CtagsConfigIsEnabled())
    let l:cscope_db = s:search_cscope_database()

    if '' == l:cscope_db
        if l:ctags_conf_usable
            call GoToDefinitionViaCtags()
        else
            echohl ErrorMsg
            echo '*** Can not find any ' . s:DB_NAME . ' !!!'
            echohl None
        endif

        return
    endif

    let l:target = expand("<cword>")

    if !l:ctags_conf_usable
        call s:go_to_definition_via_cscope(l:target)
        return
    endif

    try
        call s:go_to_definition_via_cscope(l:target)
    catch
        try
            call GoToDefinitionViaCtags()
        catch
            echohl ErrorMsg
            echo "Both cscope and ctags can't find definition of [" . l:target . "]."
            echohl None
        endtry
    endtry
endfunction

function RefreshCscopeDatabase()
    if !CscopeConfigIsEnabled()
        return
    endif

    ""let l:CSCOPE_SRC_SUFFIXES = exists('g:CSCOPE_SRC_SUFFIXES') ? g:CSCOPE_SRC_SUFFIXES : 'h,hpp,c,cc,cpp,cxx'
    ""let l:cmd_create_list = 'find -L . -iname "*.'
    ""    \. join(split(l:CSCOPE_SRC_SUFFIXES, ','), '" -o -iname "*.')
    ""    \. '" > ' . s:LIST_FILE
    let l:cmd_create_list = g:CSCOPE_VARIABLES['cmd_search_src'][1] . ' > ' . s:LIST_FILE
    let l:cmd_build_with_list = g:CSCOPE_VARIABLES['cmd_create_db_from_list'][1]
    let l:cmd_build_without_list = g:CSCOPE_VARIABLES['cmd_create_db_default_way'][1]
    let l:db_path = s:search_cscope_database()

    if '' == l:db_path
        echohl ErrorMsg
        echo '*** Can not find any ' . s:DB_NAME . ' !!!'
        echo 'You have to create it manually in proper directory by running:'
        echo '    ' . l:cmd_build_without_list
        echo 'Or:'
        echo '    # Modify arguments of "find" command according to your need.'
        echo '    ' . l:cmd_create_list
        echo '    ' . l:cmd_build_with_list
        echohl None
        return
    endif

    let l:db_dir = fnamemodify(l:db_path, ':h')
    let l:cmds = 'cd ' . l:db_dir . ' && time ('

    if filereadable(l:db_dir . '/' . s:LIST_FILE)
        echo 'Update ' . s:LIST_FILE . ' first? [y/N] '
        let l:confirm = nr2char(getchar())
        if l:confirm == 'y' || l:confirm == 'Y'
            let l:cmds = l:cmds . l:cmd_create_list . ' && '
        endif
        let l:cmds = l:cmds . l:cmd_build_with_list
    else
        let l:cmds = l:cmds . l:cmd_build_without_list
    endif

    let l:cmds = l:cmds . ' && echo "Refreshed: ' . l:db_path . '")'

    execute '!' . l:cmds
    cscope reset
endfunction

function s:search_cscope_database()
    let l:db_path = ''
    let l:db_dir = ''

    for l:i in split(fnamemodify(expand('%:p'), ':h'), '/')
        let l:db_dir = l:db_dir . '/' . l:i

        if filereadable(l:db_dir . '/' . s:DB_NAME)
            let l:db_path = l:db_dir . '/' . s:DB_NAME
        endif
    endfor

    return l:db_path
endfunction

function s:load_cscope_database()
    let l:db_path = s:search_cscope_database()

    if '' != l:db_path
        execute 'cscope add ' . l:db_path . ' ' fnamemodify(l:db_path, ':h')
    endif
endfunction

function s:action_is_missing_or_empty(action)
    return '' == a:action || '<Nop>' == a:action
endfunction

function s:save_old_key_mappings()
    for l:key in keys(s:get_new_key_mappings())
        let s:OLD_KEY_MAPPINGS[l:key] = maparg(l:key, 'n')
    endfor
endfunction

function s:restore_old_key_mappings()
    for [ l:key, l:value ] in items(s:get_new_key_mappings())
        if maparg(l:key, 'n') != l:value
            continue
        endif

        let l:action = get(s:OLD_KEY_MAPPINGS, l:key, '')

        if s:action_is_missing_or_empty(l:action)
            execute 'nunmap ' . l:key
        else
            execute 'nnoremap ' . l:key . ' ' . l:action
        endif
    endfor
endfunction

function s:get_new_key_mappings()
    return g:CSCOPE_KEY_MAPPINGS
endfunction

function s:apply_new_key_mappings()
    for [ l:key, l:value ] in items(s:get_new_key_mappings())
        if s:action_is_missing_or_empty(l:value)
            execute 'nunmap ' . l:key
        else
            execute 'nnoremap ' . l:key . ' ' . l:value
        endif
    endfor
endfunction

let s:DB_NAME = 'cscope.out'
let s:LIST_FILE = 'cscope.files'

let s:OLD_KEY_MAPPINGS = {}
" NOTE: The user is allowed to define g:CSCOPE_KEY_MAPPINGS in another script
" before this one.
" <: go to location specified by previous item of quickfix window
" >: go to location specified by next item of quickfix window
" a: find places where this symbol is assigned a value
" c: find places where the target is [c]alled
" e: find by [e]grep-style regular expression
" g: [g]oto definition
" i: find places where the target is [i]ncluded
" r: [r]efresh ctags file and/or cscope database
" s: find places using this [s]ymbol
if !exists('g:CSCOPE_KEY_MAPPINGS')
    let g:CSCOPE_KEY_MAPPINGS = {
        \ '<': ':cprevious<CR><CR>',
        \ '>': ':cnext<CR><CR>',
        \ '<Leader>a': ':cscope find a <C-R>=expand("<cword>")<CR><CR><C-o>:copen<CR>',
        \ '<Leader>c': ':cscope find c <C-R>=expand("<cword>")<CR><CR><C-o>:copen<CR>',
        \ '<Leader>e': ':cscope find e <C-R>=expand("<cword>")<CR><CR><C-o>:copen<CR>',
        \ '<Leader>g': ':call GoToDefinitionViaCscopeOrCtags()<CR>',
        \ '<Leader>i': ':cscope find i <C-R>=expand("<cfile>")<CR><CR><C-o>:copen<CR>',
        \ '<Leader>r': (
            \ exists('*RefreshCtagsFile')
            \ ? ':call RefreshCtagsFile()<CR>:call RefreshCscopeDatabase()<CR>'
            \ : ':call RefreshCscopeDatabase()<CR>'
        \ ),
        \ '<Leader>s': ':cscope find s <C-R>=expand("<cword>")<CR><CR><C-o>:copen<CR>',
    \ }
endif

let s:DEFAULT_SRC_SEARCH_CMD = 'find -L . -iname "*.h" -o -iname "*.hpp"'
    \ . ' -o -iname "*.c" -o -iname "*.cc" -o -iname "*.cpp" -o -iname "*.cxx"'
" NOTE: The user is allowed to (re-)define g:CSCOPE_VARIABLES in another script
" either before or after this one.
if !exists('g:CSCOPE_VARIABLES')
    " { 'var': [ <default value>, <working value> ] }
    let g:CSCOPE_VARIABLES = {
        \ 'cmd_search_src': [ '', s:DEFAULT_SRC_SEARCH_CMD ],
        \ 'cmd_create_db_from_list': [ '', 'cscope -bq -i ' . s:LIST_FILE ],
        \ 'cmd_create_db_default_way': [ '', 'cscope -Rbq' ],
    \ }
endif

" Specify whether to use quickfix window to show cscope results.
" For more details, run ":help cscopequickfix".
set cscopequickfix=a-,c-,d-,e-,i-,s-,t-

call s:load_cscope_database()
call EnableCscopeConfig()

"
" ================
"   CHANGE LOG
" ================
"
" >>> 2023-10-17, Man Hung-Coeng <udc577@126.com>:
"   01. Create.
"

