"
" Minimum settings for daily use.
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

" Check platform.
if has('win32') || filereadable(expand('$HOME/.cygwin'))
  let g:is_on_windows = 1
else
  let g:is_on_windows = 0
endif

" Use VIM configurations instead of those of VI.
set nocompatible

" Do NOT generate backup files.
set nobackup

" Enable BackSpace button.
set backspace=2

" DO NOT generate viminfo files.
" This is primarily used on some versions of Cygwin
" to avoid generating many annoying info files.
if is_on_windows == 1
  set viminfo=
endif

" Set space width of Tab.
set tabstop=4
set softtabstop=4

" Indent width.
set shiftwidth=4

" Replace Tab characters with spaces.
set expandtab

" Smart file type checking, which avoids bad conversions,
" e.g. Tab-to-Space expansion.
filetype plugin indent on

" Indent current line automatically as previous one.
set cindent
set autoindent

" Show line numbers.
set number

" Set file encoding priorities.
if is_on_windows == 1
  set fileencodings=gb18030,cp936,utf-8,ucs-bom,gbk,gb2312,latin1
else
  set fileencodings=utf-8,gb18030,cp936,ucs-bom,gbk,gb2312,latin1
endif

" Syntax highlight.
syntax enable
syntax on

" Highlight matched keywords.
set hlsearch

" Incremental search.
set incsearch

" Mark the column and line which the cursor locates at,
" with a colored block or line.
if is_on_windows == 1
  hi Normal ctermfg=white ctermbg=black
  hi CursorColumn cterm=NONE ctermbg=white ctermfg=red guibg=NONE guifg=NONE
  hi CursorLine cterm=NONE ctermbg=white ctermfg=red guibg=NONE guifg=NONE
else
  hi CursorColumn cterm=NONE ctermbg=black ctermfg=green guibg=NONE guifg=NONE
  hi CursorLine cterm=NONE ctermbg=black ctermfg=green guibg=NONE guifg=NONE
endif
set cursorcolumn
set cursorline

" Always show status bar, and show it with specified format.
set laststatus=2
"set statusline=[%F]%y[%{&ff}]%m%r%h%w%*%=[Line:%l/%L,Column:%c][%p%%]\ %{strftime(\"%Y/%m/%d\ %H:%M\")}
set statusline=[%F]%y[%{&ff}]%m%r%h%w%*%=[Line:%l/%L,Column:%c][%p%%]

" Show ruler which contains infomation such as line number, column number and position percentage:
" 1) If status bar is visible, show it in status bar.
" 2) Otherwise, show it at bottom line
set ruler

" Set mouse mode.
" Note: To allow select and copy operation, do not set mouse to a or i.
" set mouse=a
set mouse=v
"nmap mi :set mouse=i<cr>
"nmap ma :set mouse=a<cr>
"nmap mv :set mouse=v<cr>

" Auto-completions.
" Use iunmap command to cancel any of these in your own private script
" if you don't like them.
inoremap < <><Esc>i
inoremap ( ()<Esc>i
inoremap [ []<Esc>i
inoremap { {}<Esc>i
inoremap " ""<Esc>i
inoremap ' ''<LEFT>
inoremap ` ``<Esc>i
inoremap #in #include<Space>
inoremap #ifd #ifdef<Space>
inoremap #def #define<Space>
inoremap #en #end

"
" ================
"   CHANGE LOG
" ================
"
" >>> 2023-02-10, Man Hung-Coeng <udc577@126.com>:
"   01. Create.
"
