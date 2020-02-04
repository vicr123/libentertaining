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

#include <QDesktopServices>
#include <QShortcut>
#include "onlineerrormessages.h"
#include "onlineapi.h"
#include "questionoverlay.h"
#include "pauseoverlay.h"
#include "private/otpsetupdialog.h"
#include "private/passwordchangedialog.h"
#include "textinputoverlay.h"
#include "onlineterms.h"

struct AccountDialogPrivate {
    QNetworkAccessManager mgr;
};

AccountDialog::AccountDialog(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::AccountDialog) {
    ui->setupUi(this);
    d = new AccountDialogPrivate();

    ui->logOutButton->setProperty("type", "destructive");
    ui->mainContainer->setMaximumWidth(SC_DPI(600));
    this->setFocusProxy(ui->changeUsernameButton);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        ui->backButton->click();
    });


    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [ = ] {
        ui->backButton->click();
    });

    ui->focusBarrier->setBounceWidget(ui->logOutButton);
    ui->focusBarrier_2->setBounceWidget(ui->viewTermsAndCommunityGuidelines);

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);

    loadData();
}

AccountDialog::~AccountDialog() {
    delete ui;
    delete d;
}

void AccountDialog::on_logOutButton_clicked() {
    QuestionOverlay* question = new QuestionOverlay(this);
    question->setIcon(QMessageBox::Question);
    question->setTitle(tr("Log Out"));
    question->setText(tr("Log out of %1?").arg(OnlineApi::instance()->getLoggedInUsername()));
    question->setButtons(QMessageBox::Yes | QMessageBox::Cancel, tr("Log Out"), true);
    connect(question, &QuestionOverlay::accepted, this, [ = ](QMessageBox::StandardButton button) {
        if (button == QMessageBox::Yes) {
            //Log out of the account
            ui->backButton->click();
            OnlineApi::instance()->logOut();
        }
    });
    connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
}

void AccountDialog::on_backButton_clicked() {
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
        emit done();
    });
}

void AccountDialog::on_setup2faButton_clicked() {
    OtpSetupDialog* d = new OtpSetupDialog(this);
    connect(d, &OtpSetupDialog::done, d, &OtpSetupDialog::deleteLater);
    d->show();
}

void AccountDialog::on_changeUsernameButton_clicked() {
    bool canceled;

    QString newUsername;
    QString password;

askUsername:
    newUsername = TextInputOverlay::getText(this, tr("What's your new username?"), &canceled, newUsername);
    if (canceled) return;

    password = TextInputOverlay::getText(this, tr("Confirm the password for your account"), &canceled, "", QLineEdit::Password);
    if (canceled) goto askUsername;

    //Attempt to change the username
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/changeUsername", {
        {"username", newUsername},
        {"password", password}
    })->then([ = ](QJsonDocument doc) {
        QJsonObject obj = doc.object();

        QuestionOverlay* question = new QuestionOverlay(this);
        if (doc.object().contains("error")) {
            question->setIcon(QMessageBox::Critical);
            QString error = doc.object().value("error").toString();
            question->setTitle(tr("Changing username failed"));
            question->setText(OnlineErrorMessages::messageForCode(error, tr("Try changing your username at a later time.")));
        } else {
            question->setIcon(QMessageBox::Information);
            question->setTitle(tr("Username changed"));
            question->setText(tr("Your username has been changed."));
        }
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
        ui->usernameLabel->setText(newUsername);
    })->error([ = ](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Changing username failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
    });
}

void AccountDialog::loadData() {
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    //Load data from the server
    OnlineApi::instance()->get("/users/profile")->then([ = ](QJsonDocument doc) {
        QJsonObject obj = doc.object().value("user").toObject();

        //Get the profile picture
        int pictureSize = ui->profileInnerWidget->sizeHint().height();
        OnlineApi::instance()->profilePicture(obj.value("gravHash").toString(), pictureSize)->then([ = ](QImage image) {
            ui->profilePictureLabel->setPixmap(QPixmap::fromImage(image));
            ui->usernameLabel->setText(obj.value("username").toString());
            ui->emailLabel->setText(obj.value("email").toString());
            ui->verifyEmailWidget->setVisible(!obj.value("verified").toBool());
            ui->verifyEmailWidget->setContentsMargins(pictureSize + 6, 9, 9, 9);
            ui->stackedWidget->setCurrentWidget(ui->accountPage);
            this->setFocusProxy(ui->changeUsernameButton);
        });
    })->error([ = ](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Couldn't retrieve account information"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
        this->setFocusProxy(ui->changeUsernameButton);
    });
}

void AccountDialog::on_changePasswordButton_clicked() {
    PasswordChangeDialog* d = new PasswordChangeDialog(this);
    connect(d, &PasswordChangeDialog::done, d, &PasswordChangeDialog::deleteLater);
}

void AccountDialog::on_resendVerificationButton_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    //Resend the verification email
    OnlineApi::instance()->post("/users/resendVerification", {})->then([ = ](QJsonDocument doc) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Information);
        question->setTitle(tr("Verification Email Resent"));
        question->setText(tr("Check your mailbox for the verification code. You may have to check your spam folder."));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
    })->error([ = ](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Couldn't resend verification email"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
    });
}

void AccountDialog::on_enterVerificationButton_clicked() {
    //Ask for the verification code
    bool canceled;
    QString verificationCode = TextInputOverlay::getTextWithRegex(this, tr("Enter the verification code"), QRegularExpression("\\d{6}"), &canceled, "", tr("Enter a valid verification code"), Qt::ImhDigitsOnly);
    if (canceled) {
        return;
    }

    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    //Verify the email
    OnlineApi::instance()->post("/users/verifyEmail", {
        {"verificationCode", verificationCode}
    })->then([ = ](QJsonDocument doc) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setButtons(QMessageBox::Ok);

        QJsonObject obj = doc.object();
        if (obj.contains("error")) {
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Email Verification Failed"));
            question->setText(OnlineErrorMessages::messageForCode(obj.value("error").toString(), tr("We weren't able to verify your email. Try again later.")));
        } else {
            question->setIcon(QMessageBox::Information);
            question->setTitle(tr("Email Verified"));
            question->setText(tr("Your email has been verified. Thank you!"));

            ui->verifyEmailWidget->setVisible(false);
        }

        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
    })->error([ = ](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Couldn't verify your email."));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
    });
}

void AccountDialog::on_changeEmailButton_clicked() {
    bool canceled;

    QString newEmail;
    QString password;

askEmail:
    newEmail = TextInputOverlay::getText(this, tr("What's your new email address?"), &canceled, newEmail);
    if (canceled) return;

    password = TextInputOverlay::getText(this, tr("Confirm the password for your account"), &canceled, "", QLineEdit::Password);
    if (canceled) goto askEmail;

    //Attempt to change the username
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/changeEmail", {
        {"email", newEmail},
        {"password", password}
    })->then([ = ](QJsonDocument doc) {
        QJsonObject obj = doc.object();

        QuestionOverlay* question = new QuestionOverlay(this);
        if (doc.object().contains("error")) {
            question->setIcon(QMessageBox::Critical);
            QString error = doc.object().value("error").toString();
            question->setTitle(tr("Changing email failed"));
            question->setText(OnlineErrorMessages::messageForCode(error, tr("Try changing your email at a later time.")));
        } else {
            question->setIcon(QMessageBox::Information);
            question->setTitle(tr("Email changed"));
            question->setText(tr("Your email has been changed. Don't forget to check your email for a verification code and verify your email."));
        }
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
        ui->emailLabel->setText(newEmail);
        ui->verifyEmailWidget->setVisible(true);
    })->error([ = ](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Changing email failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->accountPage);
    });
}

void AccountDialog::on_viewTermsAndCommunityGuidelines_clicked() {
    OnlineTerms* t = new OnlineTerms(this);
    connect(t, &OnlineTerms::rejected, this, [ = ] {
        t->deleteLater();
    });
}

void AccountDialog::on_changeProfilePictureButton_clicked() {
    QDesktopServices::openUrl(QUrl("https://en.gravatar.com/gravatars/new/"));
}
