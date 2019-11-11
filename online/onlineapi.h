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
#ifndef ONLINEAPI_H
#define ONLINEAPI_H

#include <QObject>
#include <tpromise.h>
#include <QNetworkReply>
#include "libentertaining_global.h"

struct OnlineApiPrivate;
class OnlineWebSocket;
class LIBENTERTAINING_EXPORT OnlineApi : public QObject
{
        Q_OBJECT
    public:
        static OnlineApi* instance();

        tPromise<QJsonDocument>* post(QString endpoint, QJsonObject body);
        tPromise<QJsonDocument>* get(QString endpoint);
        tPromise<OnlineWebSocket*>* play(QString applicationName, QString applicationVersion, QWidget* parentWidget);

        static int httpStatusCodeFromPromiseRejection(QString rejection);
        static QString errorFromPromiseRejection(QString rejection);

    signals:

    public slots:

    private:
        explicit OnlineApi(QObject *parent = nullptr);
        static OnlineApiPrivate* d;

        QString serverHost();
        QString authorizationHeader();

        QString buildRejection(QNetworkReply* reply);
        void processReply(QNetworkReply* reply, std::function<void(QJsonDocument)> res, std::function<void(QString)> rej);
};

#endif // ONLINEAPI_H
