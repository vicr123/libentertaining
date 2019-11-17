QT += widgets gamepad svg multimedia network websockets
SHARE_APP_NAME = libentertaining

DEFINES += SETTINGS_ORGANISATION="\\\"theSuite\\\""
DEFINES += SETTINGS_APPLICATION="\\\"libentertaining\\\""
DEFINES += DEFAULT_ENTERTAINING_ONLINE_HOST="\\\"entertaining.games\\\""
DEFINES += DEFAULT_ENTERTAINING_ONLINE_HOST_IS_SECURE="true"

#Determine whether to build Discord
no-discord {
    #Don't build Discord
} else {

    macx {
        DISCORD_STATIC_PATH = /usr/local/lib/libdiscord-rpc.a

        DISCORD_LIBS = -L/usr/local/lib -ldiscord-rpc
        DISCORD_INCLUDEPATH = /usr/local/include/
    }

    unix:!macx {
        DISCORD_PATH = /usr/lib/libdiscord-rpc.so
        DISCORD_STATIC_PATH = /usr/lib/libdiscord-rpc.a

        DISCORD_LIBS = -ldiscord-rpc
    }

    win32 {
        DISCORD_STATIC_PATH = "C:/Program Files (x86)/DiscordRPC/lib/discord-rpc.lib"

        DISCORD_LIBS = -L"C:/Program Files (x86)/DiscordRPC/lib/" -ldiscord-rpc -lAdvapi32
        DISCORD_INCLUDEPATH = "C:/Program Files (x86)/DiscordRPC/include/"
    }

    exists($${DISCORD_STATIC_PATH}) || exists($${DISCORD_PATH}) || discord {
        #Build Discord
        message(Building with Discord RPC support)
        DEFINES += BUILD_DISCORD
        CONFIG += BUILD_DISCORD
        LIBS += $$DISCORD_LIBS
        INCLUDEPATH += $$DISCORD_INCLUDEPATH

        exists($${DISCORD_STATIC_PATH}) {
            DEFINES += DISCORD_STATIC
        }
    } else {
        message(Discord RPC library not found)
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
    gamepadConfiguration/buttondiagnostics.cpp \
    gamepadConfiguration/stickdiagnostics.cpp \
    gamepadConfiguration/stickdiagnosticsstickwidget.cpp \
    gamepadbuttons.cpp \
    gamepadconfigurationoverlay.cpp \
    gamepadevent.cpp \
    gamepadhud.cpp \
    gamepadlabel.cpp \
    keyboards/keyboard.cpp \
    keyboards/keyboardlayoutsdatabase.cpp \
    keyboards/keyboardlayoutsmodel.cpp \
    keyboards/layoutselect.cpp \
    keyboards/uskeyboard.cpp \
    loadoverlay.cpp \
    musicengine.cpp \
    notificationengine.cpp \
    notifications/notificationpopup.cpp \
    online/accountdialog.cpp \
    online/friendsdialog.cpp \
    online/logindialog.cpp \
    online/onlineapi.cpp \
    online/onlineerrormessages.cpp \
    online/onlinewebsocket.cpp \
    pauseoverlay.cpp \
    private/applicationeventfilter.cpp \
    private/entertainingsettings.cpp \
    private/friendpage.cpp \
    private/friendsmodel.cpp \
    private/gamepadlistener.cpp \
    private/gamepadmodel.cpp \
    private/loaddialog.cpp \
    private/loaddialogfileoptions.cpp \
    private/otpsetupdialog.cpp \
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
    gamepadConfiguration/buttondiagnostics.h \
    gamepadConfiguration/stickdiagnostics.h \
    gamepadConfiguration/stickdiagnosticsstickwidget.h \
    gamepadbuttons.h \
    gamepadconfigurationoverlay.h \
    gamepadevent.h \
    gamepadhud.h \
    gamepadlabel.h \
    keyboards/keyboard.h \
    keyboards/keyboardlayoutsdatabase.h \
    keyboards/keyboardlayoutsmodel.h \
    keyboards/layoutselect.h \
    keyboards/uskeyboard.h \
    libentertaining_global.h \
    entertaining.h \
    loadoverlay.h \
    musicengine.h \
    notificationengine.h \
    notifications/notificationpopup.h \
    online/accountdialog.h \
    online/friendsdialog.h \
    online/logindialog.h \
    online/onlineapi.h \
    online/onlineerrormessages.h \
    online/onlinewebsocket.h \
    pauseoverlay.h \
    private/applicationeventfilter.h \
    private/entertainingsettings.h \
    private/friendpage.h \
    private/friendsmodel.h \
    private/gamepadlistener.h \
    private/gamepadmodel.h \
    private/loaddialog.h \
    private/loaddialogfileoptions.h \
    private/otpsetupdialog.h \
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
onlineheader.files = online/*.h
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
    onlineheader.path = /usr/include/libentertaining/online
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
    onlineheader.path = /usr/local/include/libentertaining/online
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
    onlineheader.path = "C:/Program Files/libentertaining/include/online"
    target.path = "C:/Program Files/libentertaining/lib"
}

INSTALLS += target module header onlineheader

RESOURCES += \
    libentertaining_resources.qrc \
    libentertaining_translations.qrc

FORMS += \
    dialogueoverlay.ui \
    gamepadConfiguration/buttondiagnostics.ui \
    gamepadConfiguration/stickdiagnostics.ui \
    gamepadconfigurationoverlay.ui \
    gamepadhud.ui \
    keyboards/layoutselect.ui \
    keyboards/uskeyboard.ui \
    notifications/notificationpopup.ui \
    online/accountdialog.ui \
    online/friendsdialog.ui \
    online/logindialog.ui \
    private/friendpage.ui \
    private/loaddialog.ui \
    private/loaddialogfileoptions.ui \
    private/otpsetupdialog.ui \
    private/savedialog.ui \
    questionoverlay.ui \
    settingwidget.ui \
    textinputoverlay.ui
