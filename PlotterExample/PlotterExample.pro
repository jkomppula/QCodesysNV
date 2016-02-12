#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T09:54:30
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = PlotterExample
TEMPLATE = app

CONFIG   += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    ../qcodesysnv.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    ../qcodesysnv.h

FORMS    += mainwindow.ui
