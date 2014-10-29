#-------------------------------------------------
#
# Project created by QtCreator 2014-10-28T16:59:21
#
#-------------------------------------------------

TARGET = qt_use_appaco_lib
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    MainConsol.cpp

HEADERS += \
    MainConsol.h

LIBS += -L. -lappaco_lib
