"
" Settings of YouCompleteMe plugin.
"
" Copyright 2023-2025 Man Hung-Coeng <udc577@126.com>
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

function YcmIsInstalled()
    return 2 == exists(':YcmCompleter')
endfunction

function YcmConfigIsEnabled()
    return !exists('s:YCM_CONF_DISABLED')
        \ || '' == s:YCM_CONF_DISABLED
        \ || 0 == match(trim(string(s:YCM_CONF_DISABLED[0]), "'"), '[0nN]')
endfunction

function EnableYcmConfig()
    call s:apply_global_variables(1)

    call s:save_old_key_mappings()
    call s:apply_new_key_mappings()

    let s:YCM_CONF_DISABLED = 0
endfunction

function DisableYcmConfig()
    let s:YCM_CONF_DISABLED = 1

    call s:apply_global_variables(0)

    call s:restore_old_key_mappings()
endfunction

function GoToDefinitionIfPossible()
    if !YcmConfigIsEnabled()
        return
    endif

    " GoToDefinition should be the correct answer.
    ""let l:msg = execute('YcmCompleter GoToDefinition', '')
    " But GoToDefinitionElseDeclaration is used here because:
    " (1) I want to know where the declaration is, sometimes.
    " (2) I'll get to know that YCM fails to go to the definition
    "   before it implements project-scope navigation.
    let l:msg = execute('YcmCompleter GoToDefinitionElseDeclaration', '')

    if '' == l:msg
        return
    endif

    let l:first_line = split(l:msg, '\n')[0]

    if match(first_line, '^RuntimeError:') < 0
        return
    endif

    if match(first_line, 'Still parsing') >= 0
        return
    endif

    if exists('*CscopeConfigIsEnabled') && CscopeConfigIsEnabled()
        call GoToDefinitionViaCscopeOrCtags()
    endif
endfunction

function GoToReferencesIfPossible()
    if !YcmConfigIsEnabled()
        return
    endif

    let l:msg = execute('YcmCompleter GoToReferences', '')

    if '' == l:msg
        return
    endif

    let l:first_line = split(l:msg, '\n')[0]

    if match(first_line, '^RuntimeError:') < 0
        return
    endif

    if match(first_line, 'Still parsing') >= 0
        return
    endif

    if exists('*CscopeConfigIsEnabled') && CscopeConfigIsEnabled()
        execute 'cscope find c ' . expand("<cword>")
        copen
    endif
endfunction

function s:action_is_missing_or_empty(action)
    return '' == a:action || '<Nop>' == a:action
endfunction

function s:save_old_key_mappings()
    for l:key in keys(s:get_new_key_mappings())
        let s:OLD_KEY_MAPPINGS[l:key] = maparg(l:key, 'n')
    endfor

    " YCM is loaded after all .vimrc files,
    " which means that YcmIsInstalled() returns false
    " and all built-in key mappings of YCM are not set on startup.
    if s:action_is_missing_or_empty(s:OLD_KEY_MAPPINGS['<Leader>d']) " && YcmIsInstalled()
        let s:OLD_KEY_MAPPINGS['<Leader>d'] = ':YcmShowDetailedDiagnostic<CR>'
    endif
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
    return g:YCM_KEY_MAPPINGS
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

function s:get_global_variables()
    return g:YCM_VARIABLES
endfunction

function s:apply_global_variables(value_index)
    for [ l:key, l:value ] in items(s:get_global_variables())
        execute 'let ' . l:key . ' = ' . l:value[a:value_index]
    endfor
endfunction

set encoding=utf-8

let s:OLD_KEY_MAPPINGS = {}
" NOTE: The user is allowed to define g:YCM_KEY_MAPPINGS in another script
" before this one.
if !exists('g:YCM_KEY_MAPPINGS')
    let g:YCM_KEY_MAPPINGS = {
        \ '<': ':cprevious<CR><CR>',
        \ '>': ':cnext<CR><CR>',
        \ '<Leader>c': ':call GoToReferencesIfPossible()<CR>',
        \ '<Leader>d': ':call GoToDefinitionIfPossible()<CR>',
        \ '<Leader>h': ':YcmCompleter GoToInclude<CR>',
    \ }
endif

" NOTE: The user is allowed to define g:YCM_VARIABLES in another script
" before this one.
if !exists('g:YCM_VARIABLES')
    " { 'var': [ <default value>, <working value> ] }
    let g:YCM_VARIABLES = {
        \ 'g:ycm_confirm_extra_conf': [ 1, 0 ],
        \ 'g:ycm_add_preview_to_completeopt': [ 0, 1 ],
        \ 'g:ycm_keep_logfiles': [ 0, 0 ],
        \ 'g:ycm_key_detailed_diagnostics': [ '"<Leader>d"', '"<Leader>v"' ],
        \ 'g:ycm_max_diagnostics_to_display': [ 30, 0 ],
        \ 'g:ycm_complete_in_comments': [ 0, 1 ],
        \ 'g:ycm_clangd_args': [ '[]', '[ "--header-insertion=never" ]' ],
    \ }
endif

call EnableYcmConfig()

"
" ================
"   CHANGE LOG
" ================
"
" >>> 2023-10-17, Man Hung-Coeng <udc577@126.com>:
"   01. Create.
"
" >>> 2023-10-22, Man Hung-Coeng <udc577@126.com>:
"   01. Add GoToReferencesIfPossible() and corresponding key mappings.
"
" >>> 2024-05-21, Man Hung-Coeng <udc577@126.com>:
"   01. Customize g:ycm_clangd_args to disable auto insertion of header files.
"
" >>> 2025-09-26, Man Hung-Coeng <udc577@126.com>:
"   01. Add "set encoding=utf-8", since newer versions of YouCompleteMe
"       require UTF-8 encoding.
"

