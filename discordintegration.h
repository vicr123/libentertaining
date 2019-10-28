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
        static void makeInstance(QString appId, QString steamId);
        static DiscordIntegration* instance();

        void setPresence(QVariantMap presence);

    signals:

    public slots:

    private:
        explicit DiscordIntegration(QString appId, QString steamId);
        static DiscordIntegrationPrivate* d;
};

#endif // DISCORDINTEGRATION_H
