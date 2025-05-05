#!/bin/bash

source ${LAZY_CODING_HOME}/scripts/__import__.sh --quiet 2> /dev/null

shopt -s expand_aliases

"$@"

