MAKEFILE = QtMakefile

CONFIG += c++11 qt warn_on release
TEMPLATE = app
TARGET = ${TITLE}

FORMS += *.ui
HEADERS += ${TITLE}.hpp
SOURCES += *.cpp
QT += widgets
