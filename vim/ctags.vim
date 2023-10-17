"
" Settings of ctags plugin.
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

function CtagsConfigIsEnabled()
    return !exists('s:CTAGS_CONF_DISABLED')
        \ || '' == s:CTAGS_CONF_DISABLED
        \ || 0 == match(trim(string(s:CTAGS_CONF_DISABLED[0]), "'"), '[0nN]')
endfunction

function EnableCtagsConfig()
    let s:CTAGS_CONF_DISABLED = 0
endfunction

function DisableCtagsConfig()
    let s:CTAGS_CONF_DISABLED = 1
endfunction

function GoToDefinitionViaCtags()
    if !CtagsConfigIsEnabled()
        return
    endif

    let l:tags_path = s:search_tags_file()

    if '' == l:tags_path
        echohl ErrorMsg
        echo '*** Can not find any ' . s:TAGS_FILE . ' !!!'
        echohl None
        return
    endif

    ""call feedkeys('^]') " TODO: How to write this way??
    execute "normal \<C-]>"
endfunction

function RefreshCtagsFile()
    if !CtagsConfigIsEnabled()
        return
    endif

    let l:cmds = g:CTAGS_VARIABLES['program'][1] . ' ' . g:CTAGS_VARIABLES['extra_cmd_options'][1]
    let l:tags_path = s:search_tags_file()

    if '' != g:CTAGS_VARIABLES['prior_commands'][1]
        let l:cmds = '(' . g:CTAGS_VARIABLES['prior_commands'][1] . ') && ' . l:cmds
    endif

    if '' == l:tags_path
        echohl ErrorMsg
        echo '*** Can not find any ' . s:TAGS_FILE . ' !!!'
        echo 'You have to create it manually in proper directory by running:'
        echo '    # Add more options if necessary.'
        echo '    ' . l:cmds
        echohl None
        return
    endif

    exec '!time (cd ' . fnamemodify(l:tags_path, ':h') . ' && ' . l:cmds . ' && echo "Refreshed: ' . l:tags_path . '")'
endfunction

function s:search_tags_file()
    let l:tags_path = ''
    let l:tags_dir = ''

    for l:i in split(fnamemodify(expand('%:p'), ':h'), '/')
        let l:tags_dir = l:tags_dir . '/' . l:i

        if filereadable(l:tags_dir . '/' . s:TAGS_FILE)
            let l:tags_path = l:tags_dir . '/' . s:TAGS_FILE
        endif
    endfor

    return l:tags_path
endfunction

let s:TAGS_FILE = 'tags'

" NOTE: The user is allowed to (re-)define g:CTAGS_VARIABLES in another script
" either before or after this one.
if !exists('g:CTAGS_VARIABLES')
    " { 'var': [ <default value>, <working value> ] }
    let g:CTAGS_VARIABLES = {
        \ 'program': [ '', 'ctags' ],
        \ 'prior_commands': [ '', '' ],
        \ 'extra_cmd_options': [ '', '--exclude=".git" --exclude=".svn" --exclude=".build" --exclude="*.log" -R' ],
    \ }
endif

"
" Automatically change directory and search tags file directory by directory.
" NOTE: DO NOT miss the trailing semicolon in set-tags command!
"
set tags=tags;
set autochdir

call DisableCtagsConfig()

"
" ================
"   CHANGE LOG
" ================
"
" >>> 2023-10-17, Man Hung-Coeng <udc577@126.com>:
"   01. Create.
"

