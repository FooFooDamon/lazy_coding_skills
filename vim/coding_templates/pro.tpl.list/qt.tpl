MAKEFILE = QtMakefile

TEMPLATE = app
TARGET = ${BASENAME}
CONFIG += c++11 qt warn_on release
exists($${TARGET}.debug.pri) {
    include($${TARGET}.debug.pri) # Should contain: CONFIG += debug
}

FORMS += *.ui
HEADERS += $${TARGET}.hpp
SOURCES += *.c *.cpp
QT += widgets
