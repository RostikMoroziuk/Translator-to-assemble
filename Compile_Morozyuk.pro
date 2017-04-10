#-------------------------------------------------
#
# Project created by QtCreator 2016-12-13T22:44:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Compile_Morozyuk
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11
CONFIG += c++11


SOURCES += main.cpp\
        compiler.cpp \
    codeeditor.cpp \
    generator.cpp \
    parser.cpp \
    scaner.cpp

HEADERS  += compiler.h \
    codeeditor.h \
    scaner.h \
    generator.h \
    parser.h

FORMS    +=
