#-------------------------------------------------
#
# Project created by QtCreator 2011-11-06T21:37:41
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = uartassistant
TEMPLATE = app

include(../../../libs/qextserialport/src/qextserialport.pri)

SOURCES += main.cpp\
        dialog.cpp\
        hled.cpp \
        uploader.cpp

HEADERS  += dialog.h \
            hled.h \
            uploader.h

FORMS    += dialog.ui
