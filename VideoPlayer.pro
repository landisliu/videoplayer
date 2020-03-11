#-------------------------------------------------
#
# Project created by QtCreator 2019-10-12T22:33:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        sdlrenderwnd.cpp

HEADERS += \
        mainwindow.h \
        sdlrenderwnd.h

FORMS += \
        mainwindow.ui

#SDL2
INCLUDEPATH += F:/third_part/SDL2/include
LIBS += -LF:/third_part/SDL2/lib/x86 -lSDL2
LIBS += -LF:/third_part/SDL2/lib/x86 -lSDL2main
LIBS += -LF:/third_part/SDL2/lib/x86 -lSDL2test

#ffmpeg
INCLUDEPATH += F:/third_part/ffmpeg/include
LIBS += -LF:/third_part/ffmpeg/lib -lavcodec
LIBS += -LF:/third_part/ffmpeg/lib -lavdevice
LIBS += -LF:/third_part/ffmpeg/lib -lavfilter
LIBS += -LF:/third_part/ffmpeg/lib -lavformat
LIBS += -LF:/third_part/ffmpeg/lib -lavutil
LIBS += -LF:/third_part/ffmpeg/lib -lpostproc
LIBS += -LF:/third_part/ffmpeg/lib -lswresample
LIBS += -LF:/third_part/ffmpeg/lib -lswscale



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
