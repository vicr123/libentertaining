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
#include "logindialog.h"
#include "ui_logindialog.h"

#include <QEventLoop>
#include <QSettings>
#include <QKeyEvent>
#include <QShortcut>
#include "onlineapi.h"
#include "questionoverlay.h"
#include "musicengine.h"
#include "textinputoverlay.h"
#include "private/entertainingsettings.h"
#include "pauseoverlay.h"
#include "onlineerrormessages.h"
#include "online/onlineterms.h"

struct LoginDialogPrivate {
    QWidget* parent;
    QSettings* settings = EntertainingSettings::instance();

    QString recoveryUsername;
    QVariantMap recoveryChallenges;
};

LoginDialog::LoginDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    d = new LoginDialogPrivate();
    d->parent = parent;

    QPalette pal = ui->usernameBox->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->usernameBox->setPalette(pal);
    ui->passwordBox->setPalette(pal);
    ui->registerUsernameBox->setPalette(pal);
    ui->registerPasswordBox->setPalette(pal);
    ui->registerPasswordConfirmBox->setPalette(pal);
    ui->registerEmailBox->setPalette(pal);
    ui->frame->setPalette(pal);

    TextInputOverlay::installHandler(ui->usernameBox, tr("Username"));
    TextInputOverlay::installHandler(ui->passwordBox, tr("Password"));
    TextInputOverlay::installHandler(ui->registerUsernameBox, tr("Username"));
    TextInputOverlay::installHandler(ui->registerPasswordBox, tr("Password"));
    TextInputOverlay::installHandler(ui->registerPasswordConfirmBox, tr("Confirm Password"));
    TextInputOverlay::installHandler(ui->registerEmailBox, tr("Email Address"));

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonStart, tr("Log In"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->backButton->click();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [=] {
        ui->loginButton->click();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), ui->loginPage);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        ui->backButton->click();
    });

    ui->gamepadHud2->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud2->setButtonText(QGamepadManager::ButtonStart, tr("Register"));
    ui->gamepadHud2->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud2->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud2->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->backButton_2->click();
    });
    ui->gamepadHud2->setButtonAction(QGamepadManager::ButtonStart, [=] {
        ui->doRegisterButton->click();
    });

    QShortcut* backShortcut2 = new QShortcut(QKeySequence(Qt::Key_Escape), ui->registerPage);
    connect(backShortcut2, &QShortcut::activated, this, [=] {
        ui->backButton_2->click();
    });

    ui->gamepadHud3->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud3->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud3->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud3->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->backButton_3->click();
    });

    QShortcut* backShortcut3 = new QShortcut(QKeySequence(Qt::Key_Escape), ui->registerPage);
    connect(backShortcut3, &QShortcut::activated, this, [=] {
        ui->backButton_3->click();
    });

    this->setFocusProxy(ui->usernameBox);
    ui->loginPage->setFocusProxy(ui->usernameBox);
    ui->registerPage->setFocusProxy(ui->registerUsernameBox);
    ui->recoveryPage->setFocusProxy(ui->recoveryEmailButton);

    ui->focusBarrier->setBounceWidget(ui->usernameBox);
    ui->focusBarrier_2->setBounceWidget(ui->loginButton);
    ui->focusBarrier_3->setBounceWidget(ui->registerUsernameBox);
    ui->focusBarrier_4->setBounceWidget(ui->doRegisterButton);
    ui->focusBarrier_5->setBounceWidget(ui->recoveryEmailButton);
    ui->focusBarrier_6->setBounceWidget(ui->recoveryEmailButton);
}

LoginDialog::~LoginDialog()
{
    delete d;
    delete ui;
}

bool LoginDialog::exec()
{
    if (d->settings->contains("online/token")) return true;
    PauseOverlay::overlayForWindow(d->parent)->pushOverlayWidget(this);

    QEventLoop* loop = new QEventLoop();
    connect(this, &LoginDialog::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
    connect(this, &LoginDialog::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));

    bool success = loop->exec() == 0;
    loop->deleteLater();

    PauseOverlay::overlayForWindow(d->parent)->popOverlayWidget();

    return success;
}

void LoginDialog::on_backButton_clicked()
{
    emit rejected();
}

void LoginDialog::on_backButton_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    this->setFocusProxy(ui->usernameBox);
}

void LoginDialog::on_registerButton_clicked()
{
    ui->registerUsernameBox->setText(ui->usernameBox->text());
    ui->registerPasswordBox->setText(ui->passwordBox->text());
    ui->stackedWidget->setCurrentWidget(ui->registerPage);
    this->setFocusProxy(ui->registerUsernameBox);
}

void LoginDialog::on_doRegisterButton_clicked()
{
    QString error = "";

    if (ui->registerUsernameBox->text().isEmpty()) error = tr("Enter a new username");
    if (ui->registerPasswordBox->text().isEmpty()) error = tr("Enter a password");
    if (ui->registerPasswordConfirmBox->text().isEmpty()) error = tr("Confirm your password");
    if (ui->registerEmailBox->text().isEmpty()) error = tr("Enter your email address");

    if (ui->registerPasswordBox->text() != ui->registerPasswordConfirmBox->text()) error = tr("Check that the passwords match");

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

    //Attempt to create the user
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/create", {
                                    {"username", ui->registerUsernameBox->text()},
                                    {"password", ui->registerPasswordBox->text()},
                                    {"email", ui->registerEmailBox->text()}
                                })->then([=](QJsonDocument response) {
        QJsonObject obj = response.object();
        if (obj.contains("error")) {
            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Registration Failed"));
            question->setText(OnlineErrorMessages::messageForCode(obj.value("error").toString(), tr("We weren't able to register you. Try again later.")));
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

            ui->stackedWidget->setCurrentWidget(ui->registerPage);
        } else {
            d->settings->setValue("online/token", obj.value("token").toString());
            d->settings->sync();
            emit accepted();
        }
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Registration Failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->registerPage);
    });
}

void LoginDialog::on_loginButton_clicked()
{
    attemptLogin(ui->usernameBox->text(), ui->passwordBox->text(), "", "");
}

void LoginDialog::attemptLogin(QString username, QString password, QString otpToken, QString newPassword)
{
    QString error = "";

    if (username.isEmpty()) error = tr("Enter your username");
    if (password.isEmpty()) error = tr("Enter your password");

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

    //Attempt to log the user in
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/token", {
                                    {"username", username},
                                    {"password", password},
                                    {"otpToken", otpToken},
                                    {"newPassword", newPassword}
                                })->then([=](QJsonDocument response) {
        QJsonObject obj = response.object();
        if (obj.contains("error")) {
            QString error = obj.value("error").toString();
            if (error == "otp.required") {
                //Ask for the OTP token
                bool canceled;
                QString totpToken = TextInputOverlay::getTextWithRegex(this, tr("Enter your Two Factor Authentication code"), QRegularExpression("\\d{12}|\\d{6}"), &canceled, "", tr("Enter a valid Two Factor Authentication code"), Qt::ImhDigitsOnly);
                if (canceled) {
                    ui->stackedWidget->setCurrentWidget(ui->loginPage);
                } else {
                    QString currentPassword = password;

                    //Check to see if this is after a password reset
                    //At this point, the password has already been changed
                    if (!newPassword.isEmpty()) currentPassword = newPassword;
                    attemptLogin(username, currentPassword, totpToken, "");
                }
            } else if (error == "authentication.changePassword") {
                //Ask the user for a new password
                QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Information);
                question->setTitle(tr("Reset Password"));
                question->setText(tr("You'll need to set a new password for your account.\n\n"
                                     "Make it a good password and save it for this account. You don't want to be reusing this password."));
                question->setButtons(QMessageBox::Ok | QMessageBox::Cancel, tr("Set New Password"));
                connect(question, &QuestionOverlay::accepted, this, [=](QMessageBox::StandardButton button) {
                    question->deleteLater();
                    if (button == QMessageBox::Ok) {
                        QTimer::singleShot(1000, this, [=] {
                            QString newPassword = "";
                            bool canceled;

                            promptPassword:
                            newPassword = TextInputOverlay::getText(this, tr("Enter a new password for your account"), &canceled, "", QLineEdit::Password);
                            if (canceled) {
                                ui->stackedWidget->setCurrentWidget(ui->loginPage);
                                return;
                            }

                            TextInputOverlay::getTextWithRegex(this, tr("Confirm the new password for your account"), QRegularExpression(QRegularExpression::escape(newPassword)), &canceled, "", tr("Enter the same password"), Qt::ImhNone, QLineEdit::Password);
                            if (canceled) goto promptPassword;

                            //Attempt to reset the password
                            attemptLogin(username, password, otpToken, newPassword);
                        });
                    } else {
                        ui->stackedWidget->setCurrentWidget(ui->loginPage);
                    }
                });
                connect(question, &QuestionOverlay::rejected, this, [=] {
                    question->deleteLater();

                    ui->stackedWidget->setCurrentWidget(ui->loginPage);
                });
            } else {
                QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Login Failed"));
                question->setText(OnlineErrorMessages::messageForCode(error, tr("Check your details and try again.")));
                question->setButtons(QMessageBox::Ok);
                connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

                ui->stackedWidget->setCurrentWidget(ui->loginPage);
            }
        } else {
            d->settings->setValue("online/token", response.object().value("token").toString());
            emit accepted();
        }
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Login Failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->loginPage);
    });
}

void LoginDialog::on_viewTermsAndCommunityGuidelines_clicked()
{
    OnlineTerms* t = new OnlineTerms(this);
    connect(t, &OnlineTerms::rejected, this, [=] {
        t->deleteLater();
    });
}

void LoginDialog::on_forgotPasswordButton_clicked()
{
    bool canceled;
    d->recoveryUsername = TextInputOverlay::getText(this, tr("What's your username?"), &canceled, ui->usernameBox->text());
    if (canceled) return;

    ui->usernameBox->setText(d->recoveryUsername);

    //Prepare password recovery
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/recoverPassword", {
                                    {"username", d->recoveryUsername}
                                })->then([=](QJsonDocument response) {
        QJsonObject obj = response.object();
        if (obj.contains("error")) {
            QString error = obj.value("error").toString();
            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Password Recovery Failed"));
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

            if (error == "authentication.incorrect") {
                question->setText(tr("That username is incorrect."));
            } else {
                question->setText(OnlineErrorMessages::messageForCode(error, tr("Try again later.")));
            }

            ui->stackedWidget->setCurrentWidget(ui->loginPage);
        } else {
            d->recoveryChallenges.insert("email", obj.value("email").toString());
            ui->recoveryEmailButton->setText(tr("Send an email to %1").arg(obj.value("email").toString()));
            ui->stackedWidget->setCurrentWidget(ui->recoveryPage);
            this->setFocusProxy(ui->recoveryPage);
        }
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Password Recovery Failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->loginPage);
    });
}

void LoginDialog::on_backButton_3_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    this->setFocusProxy(ui->loginPage);
}

void LoginDialog::on_recoveryEmailButton_clicked()
{
    bool canceled;
    QString email = TextInputOverlay::getTextWithRegex(this, tr("Enter the full email address"), QRegularExpression(QRegularExpression::escape(d->recoveryChallenges.value("email").toString()).replace("\\∙\\∙\\∙", ".*")), &canceled, "", tr("Use the email address %1").arg(d->recoveryChallenges.value("email").toString()));
    if (canceled) return;

    //Attempt password recovery
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/recoverPassword", {
                                    {"username", d->recoveryUsername},
                                    {"email", email}
                                })->then([=](QJsonDocument response) {
        QJsonObject obj = response.object();

        QuestionOverlay* question = new QuestionOverlay(this);
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        if (obj.contains("error")) {
            QString error = obj.value("error").toString();
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Password Recovery Failed"));
            question->setText(OnlineErrorMessages::messageForCode(error, tr("Try again at a later time.")));
        } else {
            question->setIcon(QMessageBox::Information);
            question->setTitle(tr("Password Recovery"));
            question->setText(tr("If %1 matches the email we've got on file for your account, you'll receive an email with further instructions.").arg(email));
            ui->stackedWidget->setCurrentWidget(ui->loginPage);
            this->setFocusProxy(ui->loginPage);
        }
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Password Recovery Failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->loginPage);
        this->setFocusProxy(ui->loginPage);
    });
}
