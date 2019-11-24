/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "onlinewebsocket.h"

#include "private/entertainingsettings.h"
#include <QJsonObject>
#include <QTimer>
#include "onlineapi.h"
#include "friendsdialog.h"
#include "notificationengine.h"

struct OnlineWebSocketPrivate {
    enum State {
        Connecting,
        Connected,
        Errored
    };

    State state = Connecting;

    QWidget* parentWidget;

    QString applicationName;
    QString applicationVersion;

    QString username;

    QMap<QPair<quint64, QString>, std::function<void()>> actionActions;

    QTimer pingTimer;
    int lastPingSeq = 0;
    int nextPingSeq = 0;

    QDateTime lastPingTime;
    int currentPing = -1;
};

OnlineWebSocket::~OnlineWebSocket()
{
    delete d;
}

void OnlineWebSocket::sendJson(QJsonDocument json)
{
    this->sendTextMessage(json.toJson(QJsonDocument::Compact));
}

void OnlineWebSocket::sendJsonO(QJsonObject json)
{
    sendJson(QJsonDocument(json));
}

QString OnlineWebSocket::loggedInUsername()
{
    return d->username;
}

int OnlineWebSocket::ping()
{
    return d->currentPing;
}

void OnlineWebSocket::actionClicked(quint64 action, QString key)
{
    if (d->actionActions.contains({action, key})) {
        d->actionActions.value({action, key})();
    }
}

OnlineWebSocket::OnlineWebSocket(QString applicationName, QString applicationVersion, QWidget*parentWidget) : QWebSocket()
{
    d = new OnlineWebSocketPrivate();

    d->applicationName = applicationName;
    d->applicationVersion = applicationVersion;
    d->parentWidget = parentWidget;

    d->pingTimer.setInterval(10000);
    connect(&d->pingTimer, &QTimer::timeout, this, [=] {
        //Send a ping
        sendJsonO({
                      {"system", true},
                      {"type", "clientPing"},
                      {"seq", d->nextPingSeq}
                  });

        d->lastPingTime = QDateTime::currentDateTimeUtc();
        d->nextPingSeq++;

        if (d->nextPingSeq - d->lastPingSeq > 4) {
            //Assume we've disconnected
            this->close(QWebSocketProtocol::CloseCodeBadOperation);
        }
    });

    connect(this, &OnlineWebSocket::textMessageReceived, this, [=](QString message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isNull()) {
            QJsonObject o = doc.object();
            if (d->state == OnlineWebSocketPrivate::Connecting) {
                //This should be the handshake response
                if (o.value("status").toString() == "OK") {
                    //We're good to go!
                    d->state = OnlineWebSocketPrivate::Connected;
                    d->username = o.value("playingAs").toString();
                    d->pingTimer.start();

                    OnlineApi::setLoggedInUsername(d->username);

                    emit ready();
                } else {
                    d->state = OnlineWebSocketPrivate::Errored;
                    emit error(QAbstractSocket::OperationError);
                }
            } else {
                if (o.value("system").toBool()) {
                    //This is a system message
                    processSystemMessage(o);
                } else {
                    emit jsonMessageReceived(doc);
                }
            }
        }
    });

    connect(this, &OnlineWebSocket::connected, this, [=] {
        //Send the welcome handshake
        sendJsonO({
                      {"token", EntertainingSettings::instance()->value("online/token").toString()},
                      {"application", d->applicationName},
                      {"version", d->applicationVersion}
                 });
    });

    connect(this, &OnlineWebSocket::disconnected, this, [=] {
        d->pingTimer.stop();
    });

    connect(OnlineApi::instance(), &OnlineApi::loggedOut, this, [=] {
        //Force the connection to close
        this->close();
    });

    connect(NotificationEngine::instance(), &NotificationEngine::actionClicked, this, &OnlineWebSocket::actionClicked);
}

void OnlineWebSocket::processSystemMessage(QJsonObject obj)
{
    QString type = obj.value("type").toString();
    if (type == "clientPingReply") {
        d->lastPingSeq = obj.value("seq").toInt();
        d->currentPing = static_cast<int>(d->lastPingTime.msecsTo(QDateTime::currentDateTimeUtc()));
        emit pingChanged();
    } else if (type == "serverPing") {
        //Immediately reply
        sendJsonO({
            {"system", true},
            {"type", "serverPingReply"},
            {"seq", obj.value("seq").toInt()}
        });
    } else if (type == "notifyNewFriendRequests") {
        NotificationData notification(tr("New Friend Requests"), tr("You have friend requests pending approval"), QIcon(), {
                                          {"friendsRelations", tr("Friends and Relations")}
                                      });

        quint64 id = NotificationEngine::push(notification);
        d->actionActions.insert({id, "friendsRelations"}, [=] {
            //Open Friends and Relations
            FriendsDialog* dialog = new FriendsDialog(d->parentWidget);
            connect(dialog, &FriendsDialog::done, dialog, &FriendsDialog::deleteLater);
        });
    } else if (type == "newFriendRequest") {
        NotificationData notification(tr("New Friend Request"), tr("%1 sent you a friend request").arg(obj.value("user").toString()), QIcon(), {
                                          {"friendsRelations", tr("Friends and Relations")}
                                      });

        quint64 id = NotificationEngine::push(notification);
        d->actionActions.insert({id, "friendsRelations"}, [=] {
            //Open Friends and Relations
            FriendsDialog* dialog = new FriendsDialog(d->parentWidget);
            connect(dialog, &FriendsDialog::done, dialog, &FriendsDialog::deleteLater);
        });
    } else if (type == "friendRequestAccepted") {
        NotificationData notification(tr("Friend Request Accepted"), tr("%1 accepted your friend request").arg(obj.value("user").toString()));
        NotificationEngine::push(notification);
    }
}
