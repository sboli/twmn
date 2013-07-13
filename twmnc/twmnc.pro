CONFIG = console
TEMPLATE = app
TARGET = twmnc
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../bin/
LIBS += -lboost_program_options -lboost_system -lpthread
target.path += /usr/local/bin
INSTALLS += target
# Input
SOURCES += main.cpp
QMAKE_CXXFLAGS += -Wall -Werror -pedantic
