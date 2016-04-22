#-------------------------------------------------
#
# Project created by QtCreator 2015-09-25T11:03:42
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = defectoscop
TEMPLATE = app
#CONFIG += debug


SOURCES += main.cpp\
        widget.cpp \
    fazus.cpp \
    udpclient.cpp



HEADERS  += widget.h \
    fazus.h \
    FAZUS_DLL.h \
    udpclient.h \
    struct.h \
    bin.h \
    sleep.h \
    labelclick.h

FORMS    += widget.ui
#RC_ICONS = favicon.ico


DISTFILES +=
RC_FILE = myapp.rc
