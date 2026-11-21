# SPDX-License-Identifier: Apache-2.0

#
# Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
# All rights reserved.
#
# This file is initially generated from:
#       vim/templates/mk.tpl.list/markdown_to_pdf.tpl
# of:
#       https://github.com/FooFooDamon/lazy_coding_skills
#

.PHONY: all clean

OUTPUT_FILES := $(addsuffix .pdf, $(basename $(wildcard *.md)))

all: ${OUTPUT_FILES}

AUTHOR = ${LCS_USER}
TITLE_STR = $$(echo '$(basename $@)' | sed 's/[0-9]\+_\(.*\)/\1/')
CONTENTS_NAME = Contents
#TITLE_STYLE = \usepackage{titling}\pretitle{\begin{titlepage}\centering\vspace*{2cm}\Huge\bfseries}\posttitle{\par\vspace{1cm}\large\theauthor\par\vfill\end{titlepage}}
TITLE_STYLE = \usepackage{titling}\renewcommand{\maketitle}{\quad\vspace{2.5cm}\begin{center}\Huge{\textbf{${TITLE_STR}}}\vspace{1cm}\end{center}\begin{center}\large{${AUTHOR}}\end{center}\thispagestyle{empty}\newpage\begin{center}\tableofcontents\end{center}\thispagestyle{empty}\newpage\setcounter{page}{1}}\renewcommand{\contentsname}{${CONTENTS_NAME}}

%.pdf: %.md
	sed "/>[ \t]*${TITLE_STR}[ \t]*</d" $< | pandoc -o $@ --pdf-engine=xelatex \
		-V CJKmainfont="Noto Serif CJK SC" \
		-V geometry:"margin=1.5cm" \
		-M title="${TITLE_STR}" \
		-M author="${AUTHOR}" \
		-V header-includes="${TITLE_STYLE}"

clean:
	rm -f ${OUTPUT_FILES}
