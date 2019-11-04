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
#ifndef NOTIFICATIONENGINE_H
#define NOTIFICATIONENGINE_H

#include <QObject>
#include <QIcon>
#include <QMap>

struct NotificationData {
    NotificationData(QString title, QString text, QIcon icon = QIcon(), QMap<QString, QString> actions = QMap<QString, QString>());

    QString title;
    QString text;
    QIcon icon;
    QMap<QString, QString> actions;

    int dismissTimer = -1;
    bool dismissable = true;
};

struct NotificationEnginePrivate;
class NotificationPopup;
class NotificationEngine : public QObject
{
        Q_OBJECT
    public:
        static void setApplicationNotificationWindow(QWidget* window);

        static quint64 push(NotificationData notification);
        static void dismiss(quint64 id);

        static void focusNotifications();

    signals:

    public slots:

    private:
        explicit NotificationEngine();

        static NotificationEnginePrivate* d;
        bool eventFilter(QObject* watched, QEvent* event);

        static void ensureInstance();
        static void removePopup(NotificationPopup* popup);
        void setFirstPopup();
};

#endif // NOTIFICATIONENGINE_H
