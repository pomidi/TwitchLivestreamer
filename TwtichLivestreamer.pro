#-------------------------------------------------
#
# Project created by QtCreator 2016-01-06T13:38:49
#
#-------------------------------------------------

QT       += core gui network

CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TwtichLivestreamer

TEMPLATE = app

SOURCES +=  \
            src/main.cpp \
            src/mainwindow.cpp \
            src/download.cpp \
            src/bookmark.cpp

HEADERS  += \
            src/mainwindow.h \
            src/download.h \
            src/bookmark.h

FORMS    += \
            ui/mainwindow.ui

RESOURCES += \
            images/resources.qrc
