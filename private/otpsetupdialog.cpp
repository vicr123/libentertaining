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
#include "otpsetupdialog.h"
#include "ui_otpsetupdialog.h"

#include "online/onlineapi.h"
#include "pauseoverlay.h"
#include "questionoverlay.h"
#include "textinputoverlay.h"

struct OtpSetupDialogPrivate {
    QWidget* parent;
    QString accountPassword;
};

OtpSetupDialog::OtpSetupDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OtpSetupDialog)
{
    ui->setupUi(this);

    d = new OtpSetupDialogPrivate();
    d->parent = parent;

    ui->setupPageContainer->setMaximumWidth(SC_DPI(600));
    ui->managePageContainer->setMaximumWidth(SC_DPI(600));

    ui->turnOffTotpButton->setProperty("type", "destructive");

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    ui->gamepadHud_2->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud_2->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud_2->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud_2->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton_2->click();
    });

    ui->setupPage->setFocusProxy(ui->enterOtpTokenButton);
    ui->managePage->setFocusProxy(ui->regenerateBackupCodesButton);

    ui->focusBarrier->setBounceWidget(ui->enterOtpTokenButton);
    ui->focusBarrier_2->setBounceWidget(ui->enterOtpTokenButton);
    ui->focusBarrier_3->setBounceWidget(ui->regenerateBackupCodesButton);
    ui->focusBarrier_4->setBounceWidget(ui->turnOffTotpButton);
}

OtpSetupDialog::~OtpSetupDialog()
{
    delete d;
    delete ui;
}

void OtpSetupDialog::show()
{
    bool canceled;
    d->accountPassword = TextInputOverlay::getText(d->parent, tr("Confirm the password for your account"), &canceled, "", QLineEdit::Password);
    if (canceled) {
        emit done();
        return;
    }

    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    PauseOverlay::overlayForWindow(d->parent)->pushOverlayWidget(this);
    OnlineApi::instance()->post("/users/otp/status", {
        {"password", d->accountPassword}
    })->then([=](QJsonDocument doc) {
        QJsonObject obj = doc.object();

        if (obj.value("enabled").toBool()) {
            //2FA is already set up
            setBackupCodes(obj.value("codes").toArray());
            ui->stackedWidget->setCurrentWidget(ui->managePage);
            ui->managePage->setFocus();
        } else {
            //Set up 2FA now
            QString otpKey = obj.value("otpKey").toString();

            //Pad the OTP key
            for (int i = 1, j = 1; i < otpKey.length(); i++, j++) {
                if (j % 4 == 0) {
                    otpKey.insert(i, ' ');
                    i++;
                }
            }

            ui->keyLabel->setText(otpKey);
            ui->stackedWidget->setCurrentWidget(ui->setupPage);
            ui->setupPage->setFocus();
        }
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        if (OnlineApi::httpStatusCodeFromPromiseRejection(error) == 401) {
            question->setIcon(QMessageBox::Warning);
            question->setTitle(tr("Incorrect Details"));
            question->setText(tr("Check your password and try again."));
        } else {
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Retrieval of OTP Token Information failed"));
            question->setText(OnlineApi::errorFromPromiseRejection(error));
        }
        question->setButtons(QMessageBox::Ok);

        auto after = [=] {
            question->deleteLater();
            this->close();
        };

        connect(question, &QuestionOverlay::accepted, this, after);
        connect(question, &QuestionOverlay::rejected, this, after);
    });
}

void OtpSetupDialog::close()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=]{
        emit done();
    });
}

void OtpSetupDialog::on_backButton_clicked()
{
    this->close();
}

void OtpSetupDialog::on_backButton_2_clicked()
{
    this->close();
}

void OtpSetupDialog::on_enterOtpTokenButton_clicked()
{
    bool canceled;
    QString totpToken = TextInputOverlay::getTextWithRegex(this, tr("Enter the code displayed on your phone"), QRegularExpression("\\d{12}|\\d{6}"), &canceled, "", tr("Enter a valid Two Factor Authentication code"), Qt::ImhDigitsOnly);
    if (!canceled) {
        ui->stackedWidget->setCurrentWidget(ui->loaderPage);
        PauseOverlay::overlayForWindow(d->parent)->pushOverlayWidget(this);
        OnlineApi::instance()->post("/users/otp/enable", {
            {"otpToken", totpToken}
        })->then([=](QJsonDocument doc) {
            QJsonObject obj = doc.object();

            QuestionOverlay* question = new QuestionOverlay(this);
            if (doc.object().contains("error")) {
                question->setIcon(QMessageBox::Critical);
                QString error = doc.object().value("error").toString();
                if (error == "otp.invalidToken") {
                    question->setTitle(tr("Incorrect Details"));
                    question->setText(tr("Check the code and try again."));
                } else {
                    question->setTitle(tr("Enabling OTP Token failed"));
                    question->setText(error);
                }

                ui->stackedWidget->setCurrentWidget(ui->setupPage);
                ui->setupPage->setFocus();
            } else {
                question->setIcon(QMessageBox::Information);
                question->setTitle(tr("Two Factor Authentication enabled"));
                question->setText(tr("Two Factor Authentication has been enabled for your account."));

                setBackupCodes(obj.value("backup").toArray());
                ui->stackedWidget->setCurrentWidget(ui->managePage);
                ui->managePage->setFocus();
            }
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        })->error([=](QString error) {
            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Enabling OTP Token failed"));
            question->setText(OnlineApi::errorFromPromiseRejection(error));
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

            ui->stackedWidget->setCurrentWidget(ui->setupPage);
            ui->setupPage->setFocus();
        });
    }
}

void OtpSetupDialog::setBackupCodes(QJsonArray backupCodes)
{
    QLabel* l[10] = {
        ui->backupCode,
        ui->backupCode_2,
        ui->backupCode_3,
        ui->backupCode_4,
        ui->backupCode_5,
        ui->backupCode_6,
        ui->backupCode_7,
        ui->backupCode_8,
        ui->backupCode_9,
        ui->backupCode_10
    };

    for (int i = 0; i < 10; i++) {
        if (backupCodes.count() > i) {
            QJsonValue val = backupCodes.at(i);
            QString code;
            bool used;

            if (val.isObject()) {
                QJsonObject obj = val.toObject();
                code = obj.value("code").toString();
                used = obj.value("used").toBool();
            } else {
                code = val.toString();
                used = false;
            }

            //Pad the code key
            for (int i = 1, j = 1; i < code.length(); i++, j++) {
                if (j % 4 == 0) {
                    code.insert(i, ' ');
                    i++;
                }
            }

            QFont fnt = l[i]->font();
            fnt.setStrikeOut(used);
            l[i]->setFont(fnt);
            l[i]->setEnabled(!used);
            l[i]->setText(code);
        } else {
            l[i]->setText("");
        }
    }
}

void OtpSetupDialog::on_regenerateBackupCodesButton_clicked()
{
    QuestionOverlay* question = new QuestionOverlay(this);
    question->setIcon(QMessageBox::Question);
    question->setTitle(tr("Regenerate Backup Codes?"));
    question->setText(tr("After regenerating your backup codes, your old backup codes will be invalidated and you'll only be able to use the new backup codes."));
    question->setButtons(QMessageBox::Yes | QMessageBox::Cancel, tr("Regenerate Backup Codes"), true);
    connect(question, &QuestionOverlay::accepted, this, [=](QMessageBox::StandardButton button) {
        if (button == QMessageBox::Yes) {
            //Regenerate Codes
            ui->stackedWidget->setCurrentWidget(ui->loaderPage);
            OnlineApi::instance()->post("/users/otp/regenerate", {
                {"password", d->accountPassword}
            })->then([=](QJsonDocument doc) {
                if (doc.object().contains("error")) {
                    QuestionOverlay* question = new QuestionOverlay(this);
                    question->setIcon(QMessageBox::Critical);
                    QString error = doc.object().value("error").toString();
                    question->setTitle(tr("Backup Code Regeneration Failed"));
                    question->setText(error);
                    question->setButtons(QMessageBox::Ok);

                    connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                    connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
                } else {
                    setBackupCodes(doc.object().value("backup").toArray());
                }

                ui->stackedWidget->setCurrentWidget(ui->managePage);
                ui->managePage->setFocus();
            })->error([=](QString error) {
                QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Two Factor Authentication Removal Failed"));
                question->setText(OnlineApi::errorFromPromiseRejection(error));
                question->setButtons(QMessageBox::Ok);
                connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

                ui->stackedWidget->setCurrentWidget(ui->managePage);
                ui->managePage->setFocus();
            });
        }
    });
    connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
}

void OtpSetupDialog::on_turnOffTotpButton_clicked()
{
    QuestionOverlay* question = new QuestionOverlay(this);
    question->setIcon(QMessageBox::Question);
    question->setTitle(tr("Turn off Two Factor Authentication?"));
    question->setText(tr("After turning off Two Factor Authentication, you'll only need to log in with your password."));
    question->setButtons(QMessageBox::Yes | QMessageBox::Cancel, tr("Turn off Two Factor Authentication"), true);
    connect(question, &QuestionOverlay::accepted, this, [=](QMessageBox::StandardButton button) {
        if (button == QMessageBox::Yes) {
            //Disable TOTP
            ui->stackedWidget->setCurrentWidget(ui->loaderPage);
            OnlineApi::instance()->post("/users/otp/disable", {
                {"password", d->accountPassword}
            })->then([=](QJsonDocument doc) {
                bool close = false;
                QuestionOverlay* question = new QuestionOverlay(this);
                if (doc.object().contains("error")) {
                    question->setIcon(QMessageBox::Critical);
                    QString error = doc.object().value("error").toString();
                    question->setTitle(tr("Two Factor Authentication Removal Failed"));
                    question->setText(error);
                } else {
                    question->setIcon(QMessageBox::Information);
                    question->setTitle(tr("Two Factor Authentication Disabled"));
                    question->setText(tr("Two Factor Authentication has been disabled for your account."));
                    close = true;
                }
                question->setButtons(QMessageBox::Ok);

                auto after = [=] {
                    question->deleteLater();
                    if (close) this->close();
                };
                connect(question, &QuestionOverlay::accepted, this, after);
                connect(question, &QuestionOverlay::rejected, this, after);
            })->error([=](QString error) {
                QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Two Factor Authentication Removal Failed"));
                question->setText(OnlineApi::errorFromPromiseRejection(error));
                question->setButtons(QMessageBox::Ok);
                connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

                ui->stackedWidget->setCurrentWidget(ui->managePage);
                ui->managePage->setFocus();
            });
        }
    });
    connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
}

void OtpSetupDialog::on_stackedWidget_currentChanged(int arg1)
{
    this->setFocusProxy(ui->stackedWidget->currentWidget());
}
