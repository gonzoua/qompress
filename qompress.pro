TEMPLATE = app
CONFIG += qt consolr

include(3rdparty/zlib.pri)

HEADERS += \
    $$PWD/qzipfile.h

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/qzipfile.cpp \
