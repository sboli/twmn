#-------------------------------------------------
#
# Project created by QtCreator 2011-08-03T18:39:19
#
#-------------------------------------------------

QT       += core gui network

TARGET = twmnd
TEMPLATE = app
CONFIG += release
DESTDIR = ../bin/
LIBS += `pkg-config --libs-only-l dbus-1`
QMAKE_CXXFLAGS += `pkg-config --cflags-only-I dbus-1`
SOURCES += main.cpp\
        widget.cpp \
    settings.cpp \
    dbusinterface.cpp

HEADERS  += widget.h \
    settings.h \
    dbusinterface.h \
    message.h
