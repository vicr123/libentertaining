QT += widgets gamepad thelib svg multimedia

TEMPLATE = lib
DEFINES += LIBENTERTAINING_LIBRARY
TARGET = entertaining

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogueoverlay.cpp \
    entertaining.cpp \
    focusbarrier.cpp \
    focuspointer.cpp \
    gamepadbuttons.cpp \
    gamepadlabel.cpp \
    keyboards/keyboard.cpp \
    keyboards/uskeyboard.cpp \
    musicengine.cpp \
    pauseoverlay.cpp \
    textinputoverlay.cpp

HEADERS += \
    dialogueoverlay.h \
    focusbarrier.h \
    focuspointer.h \
    gamepadbuttons.h \
    gamepadlabel.h \
    keyboards/keyboard.h \
    keyboards/uskeyboard.h \
    libentertaining_global.h \
    entertaining.h \
    musicengine.h \
    pauseoverlay.h \
    textinputoverlay.h

DISTFILES += \
    qt_libentertaining.pri

# Install rules
header.files = *.h
module.path = $$[QMAKE_MKSPECS]/modules

unix {
    module.files = qt_libentertaining.pri
}

unix:!macx {
    target.path = /usr/lib
    header.path = /usr/include/libentertaining
    module.files = qt_libentertaining.pri
}

macx {
    CONFIG(debug, debug|release): TARGET = libentertaining_debug

    target.path = /usr/local/lib
    header.path = /usr/local/include/libentertaining
    module.files = qt_libentertaining.pri
}

win32 {
    CONFIG(debug, debug|release): TARGET = libentertainingd

    module.files = qt_thelib.pri
    header.path = "C:/Program Files/libentertaining/include"
    target.path = "C:/Program Files/libentertaining/lib"
}

INSTALLS += target module header

RESOURCES += \
    libentertaining_resources.qrc

FORMS += \
    dialogueoverlay.ui \
    keyboards/uskeyboard.ui \
    textinputoverlay.ui
