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
#ifndef OTPSETUPDIALOG_H
#define OTPSETUPDIALOG_H

#include <QWidget>

namespace Ui {
    class OtpSetupDialog;
}

struct OtpSetupDialogPrivate;
class OtpSetupDialog : public QWidget {
        Q_OBJECT

    public:
        explicit OtpSetupDialog(QWidget* parent = nullptr);
        ~OtpSetupDialog();

        void show();
        void close();

    signals:
        void done();

    private slots:
        void on_backButton_clicked();

        void on_backButton_2_clicked();

        void on_enterOtpTokenButton_clicked();

        void on_regenerateBackupCodesButton_clicked();

        void on_turnOffTotpButton_clicked();

        void on_stackedWidget_currentChanged(int arg1);

        void on_printButton_clicked();

    private:
        Ui::OtpSetupDialog* ui;
        OtpSetupDialogPrivate* d;

        void setBackupCodes(QJsonArray backupCodes);
};

#endif // OTPSETUPDIALOG_H
