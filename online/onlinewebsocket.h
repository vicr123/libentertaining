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
#ifndef ONLINEWEBSOCKET_H
#define ONLINEWEBSOCKET_H

#include <QWebSocket>
#include <QJsonDocument>
#include "libentertaining_global.h"

struct OnlineWebSocketPrivate;
class OnlineApi;
class LIBENTERTAINING_EXPORT OnlineWebSocket : public QWebSocket
{
        Q_OBJECT
    public:
        ~OnlineWebSocket();

        void sendJson(QJsonDocument json);
        void sendJsonO(QJsonObject json);

        QString loggedInUsername();

    signals:
        void jsonMessageReceived(QJsonDocument json);
        void ready();

    public slots:

    private slots:
        void actionClicked(quint64 action, QString key);

    protected:
        friend OnlineApi;
        explicit OnlineWebSocket(QString applicationName, QString applicationVersion, QWidget* parentWidget);

    private:
        OnlineWebSocketPrivate* d;
        void processSystemMessage(QJsonObject obj);
};

#endif // ONLINEWEBSOCKET_H
