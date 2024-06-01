"
" The leader script that groups other ones together.
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

function s:load_module_config_if_any(infix)
    let l:cfg_path = ''
    let l:cfg_dir = ''

    for l:i in split(fnamemodify(expand('%:p'), ':h'), '/')
        let l:cfg_dir = l:cfg_dir . '/' . l:i

        if filereadable(l:cfg_dir . '/module-specific.' . a:infix . '.vim')
            let l:cfg_path = l:cfg_dir . '/module-specific.' . a:infix . '.vim'
        elseif filereadable(l:cfg_dir . '/project-specific.' . a:infix . '.vim')
            let l:cfg_path = l:cfg_dir . '/project-specific.' . a:infix . '.vim'
        endif
    endfor

    if '' != l:cfg_path
        execute 'source ' . l:cfg_path
    endif
endfunction

call s:load_module_config_if_any('pre')

let s:THIS_PATH = expand('<sfile>:p')
let s:THIS_DIR = fnamemodify(s:THIS_PATH, ':h')
let s:PRIVATE_SCRIPT = s:THIS_DIR . '/private/' . fnamemodify(s:THIS_PATH, ':t')

if !exists('g:NO_MIN_SETTINGS') || '' == g:NO_MIN_SETTINGS || 0 == match(trim(string(g:NO_MIN_SETTINGS[0]), "'"), '[0nN]')
    execute 'source ' . s:THIS_DIR . '/minimum_settings.vim'
endif
if !exists('g:NO_CODE_TEMPLATES') || '' == g:NO_CODE_TEMPLATES || 0 == match(trim(string(g:NO_CODE_TEMPLATES[0]), "'"), '[0nN]')
    execute 'source ' . s:THIS_DIR . '/coding_templates.vim'
endif
execute 'source ' . s:THIS_DIR . '/ctags.vim'
execute 'source ' . s:THIS_DIR . '/cscope.vim'
execute 'source ' . s:THIS_DIR . '/youcompleteme.vim'
if filereadable(s:PRIVATE_SCRIPT)
    execute 'source ' . s:PRIVATE_SCRIPT
endif

call s:load_module_config_if_any('post')

"
" ================
"   CHANGE LOG
" ================
"
" >>> 2023-02-10, Man Hung-Coeng <udc577@126.com>:
"   01. Create.
"
" >>> 2023-04-12, Man Hung-Coeng <udc577@126.com>:
"   01. Turn all global variables into local ones.
"
" >>> 2023-10-17, Man Hung-Coeng <udc577@126.com>:
"   01. Support external script auto-loading.
"   02. Introduce NO_MIN_SETTINGS and NO_CODE_TEMPLATES global variables
"       to control whether to import minimum_settings.vim and
"       coding_templates.vim respectively.
"   03. Add ctags, cscope and YouCompleteMe settings.
"
" >>> 2024-06-01, Man Hung-Coeng <udc577@126.com>:
"   01. Search and load project-specific.vim if any.
"

