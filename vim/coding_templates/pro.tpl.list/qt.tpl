MAKEFILE = QtMakefile

TEMPLATE = app
TARGET = ${TITLE}
CONFIG += c++11 qt warn_on release
exists($${TARGET}.debug.pri) {
    include($${TARGET}.debug.pri) # Should contain: CONFIG += debug
}

FORMS += *.ui
HEADERS += $${TARGET}.hpp
SOURCES += *.cpp
QT += widgets
