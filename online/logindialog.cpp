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

struct LoginDialogPrivate {
    QWidget* parent;
    QSettings* settings = EntertainingSettings::instance();
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

    this->setFocusProxy(ui->usernameBox);
    ui->loginPage->setFocusProxy(ui->usernameBox);
    ui->registerPage->setFocusProxy(ui->registerUsernameBox);
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
}

void LoginDialog::on_registerButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->registerPage);
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
        d->settings->setValue("online/token", response.object().value("token").toString());
        emit accepted();
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
    QString error = "";

    if (ui->usernameBox->text().isEmpty()) error = tr("Enter your username");
    if (ui->passwordBox->text().isEmpty()) error = tr("Enter your password");

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
                                    {"username", ui->usernameBox->text()},
                                    {"password", ui->passwordBox->text()}
                                })->then([=](QJsonDocument response) {
        d->settings->setValue("online/token", response.object().value("token").toString());
        emit accepted();
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        if (OnlineApi::httpStatusCodeFromPromiseRejection(error) == 401) {
            question->setIcon(QMessageBox::Warning);
            question->setTitle(tr("Incorrect Details"));
            question->setText(tr("Check your username and password and try again."));
        } else {
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Login Failed"));
            question->setText(OnlineApi::errorFromPromiseRejection(error));
        }
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->loginPage);
    });
}
