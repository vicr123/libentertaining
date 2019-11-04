#ifndef DISCORDINTEGRATION_H
#define DISCORDINTEGRATION_H

#include <QObject>
#include <QDateTime>
#include <tpromise.h>
#include "libentertaining_global.h"

class DiscordIntegration;

struct DiscordJoinRequestCallbackProtected;
class LIBENTERTAINING_EXPORT DiscordJoinRequestCallback : public QObject {
        Q_OBJECT
    public:
        ~DiscordJoinRequestCallback();

        QString userTag();
        QUrl pictureUrl();
        tPromise<QPixmap>* profilePicture();

        void accept();
        void reject();
        void timeout();

    signals:
        void timedOut();

    protected:
        friend DiscordIntegration;

        explicit DiscordJoinRequestCallback();
        DiscordJoinRequestCallbackProtected* d;
};

struct DiscordIntegrationPrivate;
class LIBENTERTAINING_EXPORT DiscordIntegration : public QObject
{
        Q_OBJECT
    public:
        static void makeInstance(QString appId, QString steamId);
        static DiscordIntegration* instance();

        void setPresence(QVariantMap presence);

        static QString lastJoinSecret();
        static QString lastSpectateSecret();

    signals:
        void joinRequest(DiscordJoinRequestCallback* callback);
        void joinGame(QString joinSecret);
        void spectateGame(QString spectateSecret);

    public slots:

    private:
        explicit DiscordIntegration(QString appId, QString steamId);
        static DiscordIntegrationPrivate* d;
};

#endif // DISCORDINTEGRATION_H
