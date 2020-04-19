#-------------------------------------------------
#
# Project created by QtCreator 2018-03-11T22:02:36
#
#-------------------------------------------------

QT       -= core gui
TEMPLATE = lib
CONFIG +=warn_on plugin release
CONFIG-=thread exceptions qt rtti
CONFIG += c++11

INCLUDEPATH += C:\SDK\CHeaders\XPLM\
INCLUDEPATH += C:\SDK\CHeaders\Wrappers\
INCLUDEPATH += C:\SDK\CHeaders\Widgets\
INCLUDEPATH += C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17134.0\um\arm64
INCLUDEPATH += C:\Users\jeroen\Documents\testVR\GL

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS
DEFINES +=APL=0
DEFINES +=IBM=1
DEFINES +=LIN=0
DEFINES += GLEW_STATIC
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES +=XPLM302
DEFINES +=XPLM301
DEFINES +=XPLM300
DEFINES +=XPLM210
DEFINES +=XPLM200

LIBS +=-LC:\SDK\Libraries\Win
LIBS +=-lXPLM_64 -lXPWidgets_64
LIBS +=-lOpengl32 # lodbc32 lodbccp32
LIBS +=User32.lib
LIBS +=C:\Users\jeroen\Documents\testVR\glew32s.lib

TARGET = win.xpl

SOURCES += \
        testvr.cpp

HEADERS += \
        testvr.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
