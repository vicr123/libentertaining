#include "discordintegration.h"

#include <QLibrary>
#include <QDebug>

#ifdef BUILD_DISCORD
#include <discord_rpc.h>

typedef void (*InitDiscordFunction)(const char* applicationId, DiscordEventHandlers* handlers, int autoRegister, const char* optionalSteamId);
typedef void (*UpdateDiscordFunction)(const DiscordRichPresence* presence);
#endif

#define TO_CONST_CHAR(string, bufSize) ([=]() -> char* { \
        char* strBuf = new char[bufSize]; \
        sprintf(strBuf, "%s", qPrintable(string)); \
        d->bufsToDelete.append(strBuf); \
        return strBuf; \
    })()

struct DiscordIntegrationPrivate {
    DiscordIntegration* instance;

    bool discordAvailable = false;
    QList<char*> bufsToDelete;

#ifdef BUILD_DISCORD
    InitDiscordFunction Discord_Initialize;
    UpdateDiscordFunction Discord_UpdatePresence;
#endif
};

DiscordIntegrationPrivate* DiscordIntegration::d = new DiscordIntegrationPrivate();

void DiscordIntegration::makeInstance(QString appId, QString steamId)
{
    d->instance = new DiscordIntegration(appId, steamId);
}

DiscordIntegration* DiscordIntegration::instance()
{
    return d->instance;
}

void DiscordIntegration::setPresence(QVariantMap presence)
{
#ifdef BUILD_DISCORD
    if (d->discordAvailable) {
        DiscordRichPresence rp;
        memset(&rp, 0, sizeof(rp));

        if (presence.contains("state")) rp.state = TO_CONST_CHAR(presence.value("state").toString(), 128);
        if (presence.contains("details")) rp.details = TO_CONST_CHAR(presence.value("details").toString(), 128);
        if (presence.contains("startTimestamp")) rp.startTimestamp = presence.value("startTimestamp").toDateTime().toUTC().toMSecsSinceEpoch();
        if (presence.contains("endTimestamp")) rp.endTimestamp = presence.value("endTimestamp").toDateTime().toUTC().toMSecsSinceEpoch();
        if (presence.contains("largeImageKey")) rp.largeImageKey = TO_CONST_CHAR(presence.value("largeImageKey").toString(), 32);
        if (presence.contains("smallImageKey")) rp.smallImageKey = TO_CONST_CHAR(presence.value("smallImageKey").toString(), 32);
        if (presence.contains("largeImageText")) rp.largeImageText = TO_CONST_CHAR(presence.value("largeImageText").toString(), 128);
        if (presence.contains("smallImageText")) rp.smallImageText = TO_CONST_CHAR(presence.value("smallImageText").toString(), 128);
        if (presence.contains("partyId")) rp.partyId = TO_CONST_CHAR(presence.value("partyId").toString(), 128);
        if (presence.contains("partySize")) rp.partySize = presence.value("partySize").toInt();
        if (presence.contains("partyMax")) rp.partyMax = presence.value("partyMax").toInt();
        if (presence.contains("matchSecret")) rp.matchSecret = TO_CONST_CHAR(presence.value("matchSecret").toString(), 128);
        if (presence.contains("joinSecret")) rp.joinSecret = TO_CONST_CHAR(presence.value("joinSecret").toString(), 128);
        if (presence.contains("spectateSecret")) rp.spectateSecret = TO_CONST_CHAR(presence.value("spectateSecret").toString(), 128);

        qDebug() << presence;
        Discord_UpdatePresence(&rp);

        for (char* c : d->bufsToDelete) {
            delete[] c;
        }
        d->bufsToDelete.clear();
    }
#endif
}

DiscordIntegration::DiscordIntegration(QString appId, QString steamId) : QObject(nullptr)
{
    #ifdef BUILD_DISCORD
        #ifdef Q_OS_LINUX
            #ifdef DISCORD_STATIC
                d->Discord_Initialize = &::Discord_Initialize;
                d->Discord_UpdatePresence = &::Discord_UpdatePresence;
                d->discordAvailable = true;
            #else
                QLibrary* lib = new QLibrary("libdiscord-rpc.so");
                if (lib->load()) {
                    d->Discord_Initialize = reinterpret_cast<InitDiscordFunction>(lib->resolve("Discord_Initialize"));
                    d->Discord_UpdatePresence = reinterpret_cast<UpdateDiscordFunction>(lib->resolve("Discord_UpdatePresence"));
                    d->discordAvailable = true;
                }
            #endif
        #elif defined(Q_OS_MAC)
            d->Discord_Initialize = &::Discord_Initialize;
            d->Discord_UpdatePresence = &::Discord_UpdatePresence;
            d->discordAvailable = true;
        #endif

        if (d->discordAvailable) {
            DiscordEventHandlers handlers;
            memset(&handlers, 0, sizeof(handlers));
            handlers.ready = [](const DiscordUser* user) {
                qDebug() << "Discord Ready!";
            };
            handlers.errored = [](int errorCode, const char* message) {
                qDebug() << "Discord Error!";
            };
            handlers.disconnected = [](int errorCode, const char* message) {
                qDebug() << "Discord Disconnected!";
            };
            Discord_Initialize(qPrintable(appId), &handlers, true, qPrintable(steamId));
        }
    #endif
}
