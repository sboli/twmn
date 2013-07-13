QT += core gui network dbus
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
    dbusinterface.cpp \
    shortcutgrabber.cpp

HEADERS  += widget.h \
    settings.h \
    dbusinterface.h \
    message.h \
    shortcutgrabber.h
