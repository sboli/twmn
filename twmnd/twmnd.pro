#-------------------------------------------------
#
# Project created by QtCreator 2011-08-03T18:39:19
#
#-------------------------------------------------

QT       += core gui network widgets dbus
DEFINES += QT_NO_DEBUG_OUTPUT
TARGET = twmnd
TEMPLATE = app
CONFIG += debug
DESTDIR = ../bin/
LIBS += -lX11 -lXext
QMAKE_CXXFLAGS += -Wall -Werror -pedantic -Wno-long-long

target.path+=/usr/local/bin
INSTALLS += target

SOURCES += main.cpp\
        widget.cpp \
    settings.cpp \
    dbusinterface.cpp

HEADERS  += widget.h \
    settings.h \
    dbusinterface.h \
    message.h
