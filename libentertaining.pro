QT += widgets gamepad svg multimedia
SHARE_APP_NAME = libentertaining

#Determine whether to build Discord
no-discord {
    #Don't build Discord
} else {
    macx {
        exists(/usr/local/lib/libdiscord-rpc.a) || discord {
            #Build Discord
            message(Building with Discord RPC support)
            DEFINES += BUILD_DISCORD
            CONFIG += BUILD_DISCORD
            INCLUDEPATH += /usr/local/include/
            LIBS += -L/usr/local/lib -ldiscord-rpc
        }
    }

    unix!macx {
        exists(/usr/lib/libdiscord-rpc.so) || discord {
            #Build Discord
            message(Building with Discord RPC support)
            DEFINES += BUILD_DISCORD
            CONFIG += BUILD_DISCORD
        }
    }
}


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
    discordintegration.cpp \
    entertaining.cpp \
    focusbarrier.cpp \
    focuspointer.cpp \
    gamepadbuttons.cpp \
    gamepadevent.cpp \
    gamepadhud.cpp \
    gamepadlabel.cpp \
    keyboards/keyboard.cpp \
    keyboards/keyboardlayoutsdatabase.cpp \
    keyboards/uskeyboard.cpp \
    loadoverlay.cpp \
    musicengine.cpp \
    pauseoverlay.cpp \
    private/applicationeventfilter.cpp \
    private/gamepadlistener.cpp \
    private/loaddialog.cpp \
    private/loaddialogfileoptions.cpp \
    private/savedialog.cpp \
    private/saveengine.cpp \
    private/savesmodel.cpp \
    private/textinputlineedithandler.cpp \
    questionoverlay.cpp \
    saveoverlay.cpp \
    settingwidget.cpp \
    textinputoverlay.cpp

HEADERS += \
    dialogueoverlay.h \
    discordintegration.h \
    focusbarrier.h \
    focuspointer.h \
    gamepadbuttons.h \
    gamepadevent.h \
    gamepadhud.h \
    gamepadlabel.h \
    keyboards/keyboard.h \
    keyboards/keyboardlayoutsdatabase.h \
    keyboards/uskeyboard.h \
    libentertaining_global.h \
    entertaining.h \
    loadoverlay.h \
    musicengine.h \
    pauseoverlay.h \
    private/applicationeventfilter.h \
    private/gamepadlistener.h \
    private/loaddialog.h \
    private/loaddialogfileoptions.h \
    private/savedialog.h \
    private/saveengine.h \
    private/savesmodel.h \
    private/textinputlineedithandler.h \
    questionoverlay.h \
    saveoverlay.h \
    settingwidget.h \
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
    #Include required build tools
    include(/usr/share/the-libs/pri/gentranslations.pri)

    QT += thelib

    target.path = /usr/lib
    header.path = /usr/include/libentertaining
    module.files = qt_libentertaining.pri
}

macx {
    #Include required build tools
    include(/usr/local/share/the-libs/pri/gentranslations.pri)

    CONFIG(debug, debug|release): TARGET = libentertaining_debug

    INCLUDEPATH += "/usr/local/include/the-libs"
    LIBS += -L/usr/local/lib -lthe-libs -framework AppKit

    target.path = /usr/local/lib
    header.path = /usr/local/include/libentertaining
    module.files = qt_libentertaining.pri
}

win32 {
    #Include required build tools
    include(C:/Program Files/thelibs/pri/gentranslations.pri)

    CONFIG(debug, debug|release): TARGET = libentertainingd

    INCLUDEPATH += "C:/Program Files/thelibs/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs

    module.files = qt_thelib.pri
    header.path = "C:/Program Files/libentertaining/include"
    target.path = "C:/Program Files/libentertaining/lib"
}

INSTALLS += target module header

RESOURCES += \
    libentertaining_resources.qrc \
    libentertaining_translations.qrc

FORMS += \
    dialogueoverlay.ui \
    gamepadhud.ui \
    keyboards/uskeyboard.ui \
    private/loaddialog.ui \
    private/loaddialogfileoptions.ui \
    private/savedialog.ui \
    questionoverlay.ui \
    settingwidget.ui \
    textinputoverlay.ui
