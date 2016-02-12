#-------------------------------------------------
#
# Project created by QtCreator 2015-12-09T16:08:00
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TelegramSniffer
TEMPLATE = app

CONFIG   += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    ../qcodesysnv.cpp \
    popup.cpp

HEADERS  += mainwindow.h \
    ../qcodesysnv.h \
    popup.h

FORMS    += mainwindow.ui \
    popup.ui
