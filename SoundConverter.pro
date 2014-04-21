#-------------------------------------------------
#
# Project created by QtCreator
#
#-------------------------------------------------

QT += core gui widgets

TARGET = SoundConverter

TEMPLATE = app

SOURCES += main.cpp \
    soundconverter.cpp \
    running.cpp

HEADERS += soundconverter.h \
    running.h

RESOURCES += img.qrc

win32{
    LIBS += -lole32
    RC_FILE = res.rc
}
