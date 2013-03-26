#-------------------------------------------------
#
# Project created by QtCreator 2011-11-06T21:37:41
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = qupgrade
TEMPLATE = app

include(libs/qextserialport/src/qextserialport.pri)

SOURCES += src/apps/qupgrade/main.cpp\
        src/apps/qupgrade/dialog.cpp\
        src/apps/qupgrade/hled.cpp \
        src/apps/qupgrade/uploader.cpp \
        src/apps/qupgrade/qgc.cpp

HEADERS  += src/apps/qupgrade/dialog.h \
            src/apps/qupgrade/hled.h \
            src/apps/qupgrade/uploader.h \
            src/apps/qupgrade/qgc.h

INCLUDEPATH += src/apps/qupgrade

FORMS    += src/apps/qupgrade/dialog.ui
