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
#include "accountdialog.h"
#include "ui_accountdialog.h"

#include <QShortcut>
#include "onlineapi.h"
#include "questionoverlay.h"
#include "pauseoverlay.h"
#include "private/otpsetupdialog.h"

AccountDialog::AccountDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountDialog)
{
    ui->setupUi(this);

    ui->logOutButton->setProperty("type", "destructive");
    ui->mainContainer->setMaximumWidth(SC_DPI(600));
    this->setFocusProxy(ui->changeUsernameButton);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });


    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        ui->backButton->click();
    });

    ui->focusBarrier->setBounceWidget(ui->changeUsernameButton);
    ui->focusBarrier->setBounceWidget(ui->logOutButton);

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);
}

AccountDialog::~AccountDialog()
{
    delete ui;
}

void AccountDialog::on_logOutButton_clicked()
{
    QuestionOverlay* question = new QuestionOverlay(this);
    question->setIcon(QMessageBox::Question);
    question->setTitle(tr("Log Out"));
    question->setText(tr("Log out of %1?").arg(OnlineApi::instance()->getLoggedInUsername()));
    question->setButtons(QMessageBox::Yes | QMessageBox::Cancel, tr("Log Out"), true);
    connect(question, &QuestionOverlay::accepted, this, [=](QMessageBox::StandardButton button) {
        if (button == QMessageBox::Yes) {
            //Log out of the account
            ui->backButton->click();
            OnlineApi::instance()->logOut();
        }
    });
    connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
}

void AccountDialog::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit done();
    });
}

void AccountDialog::on_setup2faButton_clicked()
{
    OtpSetupDialog* d = new OtpSetupDialog(this);
    connect(d, &OtpSetupDialog::done, d, &OtpSetupDialog::deleteLater);
    d->show();
}
