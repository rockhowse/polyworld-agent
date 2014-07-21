#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T02:10:40
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = polyworld-agent
TEMPLATE = app

INCLUDEPATH += . \
               network \

SOURCES += main.cpp\
        polyworldagent.cpp \
    network/NetworkClient.cpp

HEADERS  += polyworldagent.h \
    network/NetworkClient.h

FORMS    += polyworldagent.ui

OTHER_FILES += \
    README.md
