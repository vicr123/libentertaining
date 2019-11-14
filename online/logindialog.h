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
#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>
#include "libentertaining_global.h"

namespace Ui {
    class LoginDialog;
}

struct LoginDialogPrivate;
class LIBENTERTAINING_EXPORT LoginDialog : public QWidget
{
        Q_OBJECT

    public:
        explicit LoginDialog(QWidget *parent = nullptr);
        ~LoginDialog();

        bool exec();

    signals:
        void accepted();
        void rejected();

    private slots:
        void on_backButton_clicked();

        void on_backButton_2_clicked();

        void on_registerButton_clicked();

        void on_doRegisterButton_clicked();

        void on_loginButton_clicked();

    private:
        Ui::LoginDialog *ui;
        LoginDialogPrivate* d;

        void attemptLogin(QString username, QString password, QString otpToken);
};

#endif // LOGINDIALOG_H
