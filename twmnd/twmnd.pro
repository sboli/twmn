QT       += core gui network
DEFINES += QT_NO_DEBUG_OUTPUT
TARGET = twmnd
TEMPLATE = app
CONFIG += debug
DESTDIR = ../bin/
LIBS += `pkg-config --libs-only-l dbus-1` -lX11 -lXext
QMAKE_CXXFLAGS += `pkg-config --cflags-only-I dbus-1` -Wall -Werror -pedantic -Wno-long-long

target.path+=/usr/local/bin
INSTALLS += target

SOURCES += main.cpp\
        widget.cpp \
    settings.cpp \
    dbusinterface.cpp \
    shortcutgrabber.cpp

HEADERS  += widget.h \
    settings.h \
    dbusinterface.h \
    message.h \
    shortcutgrabber.h
