MAKEFILE = QtMakefile

TEMPLATE = app
TARGET = ${BASENAME}
CONFIG += c++11 qt warn_on release
exists($${TARGET}.debug.pri) {
    include($${TARGET}.debug.pri) # Should contain: CONFIG += debug
}

CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_CFLAGS_RELEASE += -g
    !isEmpty(QMAKE_POST_LINK) {
        QMAKE_POST_LINK += &&
    }
    QMAKE_POST_LINK += objcopy --only-keep-debug $${TARGET} $${TARGET}.dbgi
    QMAKE_POST_LINK += && $${QMAKE_STRIP} --strip-all $${TARGET}
    QMAKE_POST_LINK += && objcopy --add-gnu-debuglink=$${TARGET}.dbgi $${TARGET}
}

FORMS += *.ui
HEADERS += $${TARGET}.hpp # Q*.hpp
SOURCES += *.c *.cpp
QT += widgets
