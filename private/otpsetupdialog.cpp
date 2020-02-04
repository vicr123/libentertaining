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

#include <QShortcut>
#include <QPrinter>
#include <QPrintDialog>
#include <QSvgRenderer>
#include "online/onlineapi.h"
#include "pauseoverlay.h"
#include "questionoverlay.h"
#include "textinputoverlay.h"
#include "online/onlineerrormessages.h"

struct OtpSetupDialogPrivate {
    QWidget* parent;
    QString accountPassword;

    QJsonArray backupCodes;
};

OtpSetupDialog::OtpSetupDialog(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::OtpSetupDialog) {
    ui->setupUi(this);

    d = new OtpSetupDialogPrivate();
    d->parent = parent;

    ui->setupPageContainer->setMaximumWidth(SC_DPI(600));
    ui->managePageContainer->setMaximumWidth(SC_DPI(600));

    ui->turnOffTotpButton->setProperty("type", "destructive");

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        ui->backButton->click();
    });

    ui->gamepadHud_2->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud_2->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud_2->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud_2->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        ui->backButton_2->click();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), ui->setupPage);
    connect(backShortcut, &QShortcut::activated, this, [ = ] {
        ui->backButton->click();
    });

    QShortcut* backShortcut_2 = new QShortcut(QKeySequence(Qt::Key_Escape), ui->managePage);
    connect(backShortcut_2, &QShortcut::activated, this, [ = ] {
        ui->backButton_2->click();
    });

    ui->setupPage->setFocusProxy(ui->enterOtpTokenButton);
    ui->managePage->setFocusProxy(ui->regenerateBackupCodesButton);

    ui->focusBarrier->setBounceWidget(ui->enterOtpTokenButton);
    ui->focusBarrier_2->setBounceWidget(ui->enterOtpTokenButton);
    ui->focusBarrier_3->setBounceWidget(ui->regenerateBackupCodesButton);
    ui->focusBarrier_4->setBounceWidget(ui->turnOffTotpButton);
}

OtpSetupDialog::~OtpSetupDialog() {
    delete d;
    delete ui;
}

void OtpSetupDialog::show() {
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
    })->then([ = ](QJsonDocument doc) {
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
    })->error([ = ](QString error) {
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

        auto after = [ = ] {
            question->deleteLater();
            this->close();
        };

        connect(question, &QuestionOverlay::accepted, this, after);
        connect(question, &QuestionOverlay::rejected, this, after);
    });
}

void OtpSetupDialog::close() {
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
        emit done();
    });
}

void OtpSetupDialog::on_backButton_clicked() {
    this->close();
}

void OtpSetupDialog::on_backButton_2_clicked() {
    this->close();
}

void OtpSetupDialog::on_enterOtpTokenButton_clicked() {
    bool canceled;
    QString totpToken = TextInputOverlay::getTextWithRegex(this, tr("Enter the code displayed on your phone"), QRegularExpression("\\d{12}|\\d{6}"), &canceled, "", tr("Enter a valid Two Factor Authentication code"), Qt::ImhDigitsOnly);
    if (!canceled) {
        ui->stackedWidget->setCurrentWidget(ui->loaderPage);
        PauseOverlay::overlayForWindow(d->parent)->pushOverlayWidget(this);
        OnlineApi::instance()->post("/users/otp/enable", {
            {"otpToken", totpToken}
        })->then([ = ](QJsonDocument doc) {
            QJsonObject obj = doc.object();

            QuestionOverlay* question = new QuestionOverlay(this);
            if (doc.object().contains("error")) {
                question->setIcon(QMessageBox::Critical);
                QString error = doc.object().value("error").toString();
                if (error == "otp.invalidToken") {
                    question->setTitle(tr("Incorrect Details"));
                } else {
                    question->setTitle(tr("Enabling OTP Token failed"));
                }
                question->setText(OnlineErrorMessages::messageForCode(error, tr("Try enabling Two Factor Authentication at a later time.")));

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
        })->error([ = ](QString error) {
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

void OtpSetupDialog::setBackupCodes(QJsonArray backupCodes) {
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

    d->backupCodes = backupCodes;
}

void OtpSetupDialog::on_regenerateBackupCodesButton_clicked() {
    QuestionOverlay* question = new QuestionOverlay(this);
    question->setIcon(QMessageBox::Question);
    question->setTitle(tr("Regenerate Backup Codes?"));
    question->setText(tr("After regenerating your backup codes, your old backup codes will be invalidated and you'll only be able to use the new backup codes."));
    question->setButtons(QMessageBox::Yes | QMessageBox::Cancel, tr("Regenerate Backup Codes"), true);
    connect(question, &QuestionOverlay::accepted, this, [ = ](QMessageBox::StandardButton button) {
        if (button == QMessageBox::Yes) {
            //Regenerate Codes
            ui->stackedWidget->setCurrentWidget(ui->loaderPage);
            OnlineApi::instance()->post("/users/otp/regenerate", {
                {"password", d->accountPassword}
            })->then([ = ](QJsonDocument doc) {
                if (doc.object().contains("error")) {
                    QuestionOverlay* question = new QuestionOverlay(this);
                    question->setIcon(QMessageBox::Critical);
                    QString error = doc.object().value("error").toString();
                    question->setTitle(tr("Backup Code Regeneration Failed"));
                    question->setText(OnlineErrorMessages::messageForCode(error, tr("Try regenerating your Two Factor Authentication codes at a later time.")));
                    question->setButtons(QMessageBox::Ok);

                    connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                    connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
                } else {
                    setBackupCodes(doc.object().value("backup").toArray());
                }

                ui->stackedWidget->setCurrentWidget(ui->managePage);
                ui->managePage->setFocus();
            })->error([ = ](QString error) {
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

void OtpSetupDialog::on_turnOffTotpButton_clicked() {
    QuestionOverlay* question = new QuestionOverlay(this);
    question->setIcon(QMessageBox::Question);
    question->setTitle(tr("Turn off Two Factor Authentication?"));
    question->setText(tr("After turning off Two Factor Authentication, you'll only need to log in with your password."));
    question->setButtons(QMessageBox::Yes | QMessageBox::Cancel, tr("Turn off Two Factor Authentication"), true);
    connect(question, &QuestionOverlay::accepted, this, [ = ](QMessageBox::StandardButton button) {
        if (button == QMessageBox::Yes) {
            //Disable TOTP
            ui->stackedWidget->setCurrentWidget(ui->loaderPage);
            OnlineApi::instance()->post("/users/otp/disable", {
                {"password", d->accountPassword}
            })->then([ = ](QJsonDocument doc) {
                bool close = false;
                QuestionOverlay* question = new QuestionOverlay(this);
                if (doc.object().contains("error")) {
                    question->setIcon(QMessageBox::Critical);
                    QString error = doc.object().value("error").toString();
                    question->setTitle(tr("Two Factor Authentication Removal Failed"));
                    question->setText(OnlineErrorMessages::messageForCode(error, tr("Try removing Two Factor Authentication at a later time.")));
                } else {
                    question->setIcon(QMessageBox::Information);
                    question->setTitle(tr("Two Factor Authentication Disabled"));
                    question->setText(tr("Two Factor Authentication has been disabled for your account."));
                    close = true;
                }
                question->setButtons(QMessageBox::Ok);

                auto after = [ = ] {
                    question->deleteLater();
                    if (close) this->close();
                };
                connect(question, &QuestionOverlay::accepted, this, after);
                connect(question, &QuestionOverlay::rejected, this, after);
            })->error([ = ](QString error) {
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

void OtpSetupDialog::on_stackedWidget_currentChanged(int arg1) {
    this->setFocusProxy(ui->stackedWidget->currentWidget());
}

void OtpSetupDialog::on_printButton_clicked() {
    QPrinter* printer = new QPrinter();
    printer->setPageMargins(1, 1, 1, 1, QPrinter::Inch);

    QPrintDialog* dialog = new QPrintDialog(printer);
    dialog->setWindowTitle(tr("Print Backup Codes"));
    connect(dialog, QOverload<QPrinter*>::of(&QPrintDialog::accepted), this, [ = ]() {
        QPainter* painter = new QPainter(printer);

        //Print cool stuff! Let's go!
        QFont bodyFont = this->font();
        QFont subtitleFont = this->font();
        subtitleFont.setBold(true);
        QFont titleFont = this->font();
        titleFont.setPointSize(30);
        QFont codeFont = this->font();
        codeFont.setPointSize(15);

        auto printText = [ = ](QString text, QFont font, int top) {
            QFontMetrics metrics(font, printer);

            QRect textRect;
            textRect.setSize(printer->pageRect().size());
            textRect.setTop(top);
            textRect.moveLeft(0);

            textRect = metrics.boundingRect(textRect, Qt::TextWordWrap, text);

            painter->setFont(font);
            painter->drawText(textRect, text);

            return textRect.bottom();
        };

        QFontMetrics titleMetrics(titleFont, printer);
        int currentY;

        //Draw the Entertaining Games icon
        QSvgRenderer iconRenderer(QStringLiteral(":/libentertaining/icons/icon.svg"));
        QRect iconRect;
        iconRect.setSize(QSize(titleMetrics.height(), titleMetrics.height()));
        iconRect.moveTopLeft(QPoint(0, 0));
        iconRenderer.render(painter, iconRect);

        //Draw the Entertaining Games title
        QString logoText = tr("Entertaining Games");
        QRect logoTextRect;
        logoTextRect.setSize(QSize(titleMetrics.horizontalAdvance(logoText), titleMetrics.height()));
        logoTextRect.moveTop(0);
        logoTextRect.moveLeft(iconRect.right() + 9);
        painter->setFont(titleFont);
        painter->drawText(logoTextRect, logoText);

        //Draw the title text
        currentY = printText(tr("Two Factor Authentication Backup Codes"), codeFont, logoTextRect.bottom() + 9);

        //Draw a header line
        currentY += 9;
        painter->setPen(Qt::black);
        painter->drawLine(0, currentY, printer->pageRect().width(), currentY);
        currentY++;

        currentY = printText(tr("Hey there,").append("\n\n")
                .append(tr("Your backup codes are displayed below. Keep them in a safe place.")) //Keep in sync with the UI
                .append("\n\n")
                .append(tr("Each backup code can only be used once, so it's a good idea to "
                        "cross each one out as you use it."))
                .append("\n\n")
                .append(tr("This page was printed on %1, so if you've regenerated your backup "
                        "codes since then, these ones may not be the correct ones to use.")
                    .arg(QLocale().toString(QDateTime::currentDateTime(), "ddd, dd MMM yyyy"))),
                bodyFont, currentY + 9);

        //Start printing the backup codes!
        auto printBackupCode = [ = ](QJsonValue val, int number, int startY) {
            painter->save();

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

            QFont font = codeFont;
            font.setStrikeOut(used);

            painter->setOpacity(used ? 0.5 : 1);

            //Calculate where we need to put this text
            QFontMetrics metrics(font, printer);

            QRect textRect;
            textRect.setWidth(printer->pageRect().size().width() / 2);
            textRect.setHeight(metrics.height());
            textRect.moveTop(startY + ((metrics.height() + 9) * (number / 2)));
            textRect.moveLeft(textRect.width() * (number % 2));

            painter->setFont(font);
            painter->drawText(textRect, Qt::AlignCenter, code);

            painter->restore();

            return textRect.bottom();
        };

        int newBottom = currentY;
        currentY += 9;
        for (int i = 0; i < d->backupCodes.count(); i++) {
            newBottom = printBackupCode(d->backupCodes.at(i), i, currentY);
        }
        currentY = newBottom;

        currentY = printText(tr("Need more codes?").toUpper(), subtitleFont, currentY + 9);
        currentY = printText(tr("Generate more codes in any Entertaining Games application. "
                    "These codes will be invalidated when you do."),
                bodyFont, currentY + 9);

        painter->end();

        dialog->deleteLater();
        delete printer;
    });
    connect(dialog, &QPrintDialog::rejected, this, [ = ] {
        dialog->deleteLater();
        delete printer;
    });
    dialog->open();
}
