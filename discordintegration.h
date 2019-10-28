#ifndef DISCORDINTEGRATION_H
#define DISCORDINTEGRATION_H

#include <QObject>
#include <QDateTime>
#include "libentertaining_global.h"

struct DiscordIntegrationPrivate;
class LIBENTERTAINING_EXPORT DiscordIntegration : public QObject
{
        Q_OBJECT
    public:
        struct RichPresence {
            QString state;   /* max 128 bytes */
            QString details; /* max 128 bytes */
            QDateTime startTimestamp;
            QDateTime endTimestamp;
            QString largeImageKey;  /* max 32 bytes */
            QString largeImageText; /* max 128 bytes */
            QString smallImageKey;  /* max 32 bytes */
            QString smallImageText; /* max 128 bytes */
            QString partyId;        /* max 128 bytes */
            int partySize;
            int partyMax;
            QString matchSecret;    /* max 128 bytes */
            QString joinSecret;     /* max 128 bytes */
            QString spectateSecret; /* max 128 bytes */
            qint8 instance;
        };

        static void makeInstance(QString appId, QString steamId);
        static DiscordIntegration* instance();

        void setPresence(RichPresence presence);

    signals:

    public slots:

    private:
        explicit DiscordIntegration(QString appId, QString steamId);
        static DiscordIntegrationPrivate* d;
};

#endif // DISCORDINTEGRATION_H
