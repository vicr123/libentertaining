#include "discordintegration.h"

#include <QLibrary>
#include <QDebug>

#ifdef BUILD_DISCORD
#include <discord_rpc.h>

typedef void (*InitDiscordFunction)(const char* applicationId, DiscordEventHandlers* handlers, int autoRegister, const char* optionalSteamId);
typedef void (*UpdateDiscordFunction)(const DiscordRichPresence* presence);
#endif

struct DiscordIntegrationPrivate {
    DiscordIntegration* instance;

    bool discordAvailable = false;

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

void DiscordIntegration::setPresence(DiscordIntegration::RichPresence presence)
{
#ifdef BUILD_DISCORD
    if (d->discordAvailable) {
        DiscordRichPresence rp;
        memset(&rp, 0, sizeof(rp));

        rp.state = qPrintable(presence.state);
        rp.details = qPrintable(presence.details);
        rp.startTimestamp = presence.startTimestamp.toUTC().toMSecsSinceEpoch();
        rp.endTimestamp = presence.endTimestamp.toUTC().toMSecsSinceEpoch();
        rp.largeImageKey = qPrintable(presence.largeImageKey);
        rp.smallImageKey = qPrintable(presence.smallImageKey);
        rp.largeImageText = qPrintable(presence.largeImageText);
        rp.smallImageText = qPrintable(presence.smallImageText);
        rp.partyId = qPrintable(presence.partyId);
        rp.partySize = presence.partySize;
        rp.partyMax = presence.partyMax;
        rp.matchSecret = qPrintable(presence.matchSecret);
        rp.joinSecret = qPrintable(presence.joinSecret);
        rp.spectateSecret = qPrintable(presence.spectateSecret);
        rp.instance = presence.instance;

        Discord_UpdatePresence(&rp);
    }
#endif
}

DiscordIntegration::DiscordIntegration(QString appId, QString steamId) : QObject(nullptr)
{
    #ifdef BUILD_DISCORD
        #ifdef Q_OS_LINUX
            QLibrary* lib = new QLibrary("libdiscord-rpc.so");
            if (lib->load()) {
                d->Discord_Initialize = reinterpret_cast<InitDiscordFunction>(lib->resolve("Discord_Initialize"));
                d->Discord_UpdatePresence = reinterpret_cast<UpdateDiscordFunction>(lib->resolve("Discord_UpdatePresence"));
                d->discordAvailable = true;
            }
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
