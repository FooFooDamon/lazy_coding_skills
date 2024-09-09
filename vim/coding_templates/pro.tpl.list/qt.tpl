MAKEFILE = QtMakefile

CONFIG += c++11 qt warn_on release
exists(${TITLE}.debug.pri) {
    include(${TITLE}.debug.pri) # Should contain: CONFIG += debug
}
TEMPLATE = app
TARGET = ${TITLE}

FORMS += *.ui
HEADERS += ${TITLE}.hpp
SOURCES += *.cpp
QT += widgets
