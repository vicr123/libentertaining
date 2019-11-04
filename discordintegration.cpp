#include "discordintegration.h"

#include <QLibrary>
#include <QDebug>
#include <QTimer>
#include <QUrl>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#ifdef BUILD_DISCORD
#include <discord_rpc.h>

typedef void (*InitDiscordFunction)(const char* applicationId, DiscordEventHandlers* handlers, int autoRegister, const char* optionalSteamId);
typedef void (*UpdateDiscordFunction)(const DiscordRichPresence* presence);
typedef void (*DiscordRespondFunction)(const char* userid, int reply);
typedef void (*DiscordRunCallbacksFunction)(void);
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

    QString joinSecret;
    QString spectateSecret;

    QTimer* discordCallbacksTimer;

#ifdef BUILD_DISCORD
    InitDiscordFunction Discord_Initialize;
    UpdateDiscordFunction Discord_UpdatePresence;
    DiscordRespondFunction Discord_Respond;
    DiscordRunCallbacksFunction Discord_RunCallbacks;
#endif
};

struct DiscordJoinRequestCallbackProtected {
    QString profilePicture;
    QString username;
    QString discriminator;
    QString userId;

    std::function<void()> acceptCallback;
    std::function<void()> rejectCallback;
    std::function<void()> timeoutCallback;
};

DiscordJoinRequestCallback::~DiscordJoinRequestCallback()
{
    delete d;
}

QString DiscordJoinRequestCallback::userTag()
{
    return QStringLiteral("%1#%2").arg(d->username).arg(d->discriminator);
}

QUrl DiscordJoinRequestCallback::pictureUrl()
{
    if (d->profilePicture.isEmpty()) {
        return QUrl(QStringLiteral("https://cdn.discordapp.com/embed/avatars/%1.png").arg(d->discriminator.toInt() % 5));
    } else {
        return QUrl(QStringLiteral("https://cdn.discordapp.com/avatars/%1/%2.png").arg(d->userId).arg(d->profilePicture));
    }
}

tPromise<QPixmap>*DiscordJoinRequestCallback::profilePicture()
{
    return new tPromise<QPixmap>([=](std::function<void(QPixmap)> successFunction, std::function<void(QString)> failureFunction) {
        QNetworkAccessManager* mgr = new QNetworkAccessManager();
        QNetworkReply* reply = mgr->get(QNetworkRequest(this->pictureUrl()));
        connect(reply, &QNetworkReply::finished, this, [=] {
            QPixmap pixmap;
            if (pixmap.loadFromData(reply->readAll())) {
                successFunction(pixmap);
            } else {
                failureFunction("Couldn't load pixmap");
            }
            reply->deleteLater();
            mgr->deleteLater();
        });
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, [=](QNetworkReply::NetworkError error) {
            qDebug() << error;
        });
    });
}

void DiscordJoinRequestCallback::accept()
{
    d->acceptCallback();
}

void DiscordJoinRequestCallback::reject()
{
    d->rejectCallback();
}

void DiscordJoinRequestCallback::timeout()
{
    d->timeoutCallback();
    emit timedOut();
}

DiscordJoinRequestCallback::DiscordJoinRequestCallback() : QObject(nullptr)
{
    d = new DiscordJoinRequestCallbackProtected();
}

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

QString DiscordIntegration::lastJoinSecret()
{
    return d->joinSecret;
}

QString DiscordIntegration::lastSpectateSecret()
{
    return d->spectateSecret;
}

DiscordIntegration::DiscordIntegration(QString appId, QString steamId) : QObject(nullptr)
{
    #ifdef BUILD_DISCORD
        #ifdef DISCORD_STATIC
            d->Discord_Initialize = &::Discord_Initialize;
            d->Discord_UpdatePresence = &::Discord_UpdatePresence;
            d->Discord_Respond = &::Discord_Respond;
            d->Discord_RunCallbacks = &::Discord_RunCallbacks;
            d->discordAvailable = true;
        #else
            QLibrary* lib;
            #ifdef Q_OS_LINUX
                lib = new QLibrary("libdiscord-rpc.so");
            #elif defined(Q_OS_MAC)
                lib = new QLibrary("libdiscord-rpc.dylib");
            #elif defined(Q_OS_WIN)
                lib = new QLibrary("discord-rpc.dll");
            #endif

            if (lib->load()) {
                d->Discord_Initialize = reinterpret_cast<InitDiscordFunction>(lib->resolve("Discord_Initialize"));
                d->Discord_UpdatePresence = reinterpret_cast<UpdateDiscordFunction>(lib->resolve("Discord_UpdatePresence"));
                d->Discord_Respond = reinterpret_cast<DiscordRespondFunction>(lib->resolve("Discord_Respond"));
                d->Discord_RunCallbacks = reinterpret_cast<DiscordRunCallbacksFunction>(lib->resolve("Discord_RunCallbacks"));
                d->discordAvailable = true;
            }
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
            handlers.joinRequest = [](const DiscordUser* user) {
                DiscordJoinRequestCallback* callback = new DiscordJoinRequestCallback();
                callback->d->profilePicture = QString::fromLatin1(user->avatar);
                callback->d->userId = QString::fromLatin1(user->userId);
                callback->d->username = QString::fromLatin1(user->username);
                callback->d->discriminator = QString::fromLatin1(user->discriminator);

                callback->d->acceptCallback = [=] {
                    d->Discord_Respond(callback->d->userId.toLatin1(), DISCORD_REPLY_YES);
                };
                callback->d->rejectCallback = [=] {
                    d->Discord_Respond(callback->d->userId.toLatin1(), DISCORD_REPLY_NO);
                };
                callback->d->timeoutCallback = [=] {
                    d->Discord_Respond(callback->d->userId.toLatin1(), DISCORD_REPLY_IGNORE);
                };

                emit d->instance->joinRequest(callback);
            };
            handlers.joinGame = [](const char* joinSecret) {
                d->joinSecret = QString::fromLatin1(joinSecret);
                qDebug() << "Join game" << d->joinSecret;
                emit d->instance->joinGame(d->joinSecret);
            };
            handlers.spectateGame = [](const char* spectateSecret) {
                d->spectateSecret = QString::fromLatin1(spectateSecret);
                emit d->instance->joinGame(d->spectateSecret);
            };
            Discord_Initialize(qPrintable(appId), &handlers, true, qPrintable(steamId));

            d->discordCallbacksTimer = new QTimer();
            d->discordCallbacksTimer->setInterval(1000);
            connect(d->discordCallbacksTimer, &QTimer::timeout, this, [=] {
                d->Discord_RunCallbacks();
            });
            d->discordCallbacksTimer->start();
        }
    #endif
}
