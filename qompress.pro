TEMPLATE = app
CONFIG += qt consolr
mac:DEFINES += USE_FILE32API

include(3rdparty/zlib.pri)

HEADERS += \
    $$PWD/qzipfile.h

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/qzipfile.cpp \
