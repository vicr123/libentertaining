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
#ifndef FRIENDPAGE_H
#define FRIENDPAGE_H

#include <QWidget>

namespace Ui {
    class FriendPage;
}

struct FriendPagePrivate;
class FriendPage : public QWidget
{
        Q_OBJECT

    public:
        explicit FriendPage(QWidget *parent = nullptr);
        ~FriendPage();

        void setActiveUser(QString username, QString friendStatus);

    signals:
        void blockUi(bool block);
        void reloadFriendsModel();

    private slots:
        void on_acceptIncomingButton_clicked();

        void on_declineIncomingButton_clicked();

        void on_removeFriendButton_clicked();

        void on_retractRequestButton_clicked();

    private:
        Ui::FriendPage *ui;
        FriendPagePrivate* d;

        void respondToFriendRequest(QString response);
};

#endif // FRIENDPAGE_H
