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
#include "passwordchangedialog.h"
#include "ui_passwordchangedialog.h"

#include <QJsonDocument>
#include <QShortcut>
#include "online/onlineapi.h"
#include "online/onlineerrormessages.h"
#include "musicengine.h"
#include "textinputoverlay.h"
#include "questionoverlay.h"
#include "pauseoverlay.h"

PasswordChangeDialog::PasswordChangeDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PasswordChangeDialog)
{
    ui->setupUi(this);

    QPalette pal = ui->newPasswordBox->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->newPasswordBox->setPalette(pal);
    ui->confirmPasswordBox->setPalette(pal);

    TextInputOverlay::installHandler(ui->newPasswordBox, tr("New Password"));
    TextInputOverlay::installHandler(ui->confirmPasswordBox, tr("Confirm Password"));

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonStart, tr("Change Password"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->backButton->click();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [=] {
        ui->changePasswordButton->click();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        ui->backButton->click();
    });

    this->setFocusProxy(ui->newPasswordBox);
    ui->focusBarrier->setBounceWidget(ui->newPasswordBox);
    ui->focusBarrier_2->setBounceWidget(ui->changePasswordButton);

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);
}

PasswordChangeDialog::~PasswordChangeDialog()
{
    delete ui;
}

void PasswordChangeDialog::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit done();
    });
}

void PasswordChangeDialog::on_changePasswordButton_clicked()
{
    QString error = "";

    if (ui->newPasswordBox->text().isEmpty()) error = tr("Enter a new password");
    if (ui->confirmPasswordBox->text().isEmpty()) error = tr("Confirm your password");

    if (ui->newPasswordBox->text() != ui->confirmPasswordBox->text()) error = tr("Check that the passwords match");

    if (!error.isEmpty()) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Information);
        question->setTitle(tr("Check your input"));
        question->setText(error);
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        return;
    }

    //Ask for the user's old password
    bool canceled;
    QString currentPassword = TextInputOverlay::getText(this, tr("Confirm the password for your account"), &canceled, "", QLineEdit::Password);
    if (canceled) {
        return;
    }

    //Attempt to change the password
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/changePassword", {
                                    {"password", currentPassword},
                                    {"newPassword", ui->newPasswordBox->text()},
                                })->then([=](QJsonDocument response) {
        QJsonObject obj = response.object();

        QuestionOverlay* question = new QuestionOverlay(this);
        question->setButtons(QMessageBox::Ok);

        if (obj.contains("error")) {
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Chaning Password Failed"));
            question->setText(OnlineErrorMessages::messageForCode(obj.value("error").toString(), tr("We weren't able to change your password. Try again later.")));

            ui->stackedWidget->setCurrentWidget(ui->changePasswordPage);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        } else {
            question->setIcon(QMessageBox::Information);
            question->setTitle(tr("Password Changed"));
            question->setText(tr("Your password has been changed."));

            auto after = [=] {
                question->deleteLater();
                ui->backButton->click();
            };

            connect(question, &QuestionOverlay::accepted, this, after);
            connect(question, &QuestionOverlay::rejected, this, after);
        }

    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Changing Password Failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->changePasswordPage);
    });
}
