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
#include "onlineterms.h"
#include "ui_onlineterms.h"

#include <QDesktopServices>
#include "onlineapi.h"
#include "pauseoverlay.h"
#include "questionoverlay.h"

struct OnlineTermsPrivate {
    QWidget* parent;
    QNetworkAccessManager mgr;

    bool isTermsChanged = false;
};

OnlineTerms::OnlineTerms(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineTerms)
{
    d = new OnlineTermsPrivate();
    d->parent = parent;

    this->init();
}

OnlineTerms::~OnlineTerms()
{
    delete ui;
}

void OnlineTerms::on_viewCommunityGuidelinesButton_clicked()
{
    view(tr("Community Guidelines"), "communityguidelines");
}

void OnlineTerms::on_backButton_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}

OnlineTerms::OnlineTerms(QWidget*parent, bool isTermsChanged) : QWidget(parent), ui(new Ui::OnlineTerms)
{
    d = new OnlineTermsPrivate();
    d->parent = parent;
    d->isTermsChanged = isTermsChanged;

    this->init();
}

void OnlineTerms::init()
{
    ui->setupUi(this);

    ui->mainWidget->setMaximumWidth(SC_DPI(600));
    ui->viewPageTextBrowser->setMaximumWidth(SC_DPI(600));

    if (QLocale().language() == QLocale::English || QLocale().language() == QLocale::C) {
        ui->localeWarning->setVisible(false);
    } else {
        QPalette pal = ui->localeWarningFrame->palette();
        pal.setColor(QPalette::Window, QColor(255, 100, 0));
        pal.setColor(QPalette::WindowText, Qt::white);
        ui->localeWarningFrame->setPalette(pal);

        QString locWarning = tr("Warning").toUpper();
        if (locWarning == "WARNING") {
            ui->localeWarningTitle->setText("WARNING");
        } else {
            ui->localeWarningTitle->setText("WARNING / " + locWarning);
        }

        const char* localeWarningText = QT_TR_NOOP("The presiding translation for these documents is English; "
                                                   "only the English version of these documents will be taken into account should any discreapency occur. "
                                                   "However, should a translation be available, we will show it for your convenience, in the hope that you'll "
                                                   "be able to understand the translation better.");

        ui->localeWarningEnglish->setText(localeWarningText);

        if (localeWarningText == tr(localeWarningText)) {
            ui->localeWarningOther->setVisible(false);
        } else {
            ui->localeWarningOther->setText(tr(localeWarningText));
        }
    }

    if (d->isTermsChanged) {
        ui->descriptionLabel->setText(tr("The Terms and Conditions or Community Guidelines have been updated. To continue playing online, you'll need to read and agree to the new documents."));
        ui->focusBarrier_2->setBounceWidget(ui->logoutButton);
    } else {
        ui->descriptionLabel->setText(tr("The Terms and Conditions and Community Guidelines govern your use of the Entertaining Games services. By creating an account and using the services, you agree to these documents."));
        ui->acceptButton->setVisible(false);
        ui->logoutButton->setVisible(false);
        ui->focusBarrier_2->setBounceWidget(ui->viewPrivacyPolicyButton);
    }

    this->setFocusProxy(ui->viewCommunityGuidelinesButton);
    ui->mainPage->setFocusProxy(ui->viewCommunityGuidelinesButton);
    ui->viewPage->setFocusProxy(ui->viewPageTextBrowser);
    ui->logoutButton->setProperty("type", "destructive");

    ui->focusBarrier->setBounceWidget(ui->viewCommunityGuidelinesButton);
    ui->focusBarrier_3->setBounceWidget(ui->viewPageTextBrowser);
    ui->focusBarrier_4->setBounceWidget(ui->viewPageTextBrowser);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    ui->gamepadHud_2->setButtonText(QGamepadManager::ButtonB, tr("Back"));
    ui->gamepadHud_2->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton_2->click();
    });

    PauseOverlay::overlayForWindow(d->parent)->pushOverlayWidget(this);
}

void OnlineTerms::view(QString title, QString document)
{
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    ui->viewPageTitle->setText(title);

    QNetworkRequest cgRequest(OnlineApi::instance()->urlForPath(QStringLiteral("/info/%1").arg(document)));
    cgRequest.setRawHeader("Accept-Language", QLocale().bcp47Name().toUtf8());
    QNetworkReply* cgReply = d->mgr.get(cgRequest);
    connect(cgReply, &QNetworkReply::finished, this, [=] {
        if (cgReply->error() == QNetworkReply::NoError) {
            ui->viewPageTextBrowser->setHtml(cgReply->readAll());
        } else {
            ui->viewPageTextBrowser->setText(tr("Encountered an error trying to load the community guidelines. Read them online at <a href=\"%1\">%1</a>")
                                                        .arg(OnlineApi::instance()->urlForPath("/info/communityguidelines").toString()));
        }

        ui->stackedWidget->setCurrentWidget(ui->viewPage);
        ui->viewPageTextBrowser->setFocus();
    });
}

void OnlineTerms::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit rejected();
    });
}

void OnlineTerms::on_acceptButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->loaderPage);
    OnlineApi::instance()->post("/users/acceptTerms", {})->then([=](QJsonDocument reply) {
        PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
            emit accepted();
        });
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Acceptance Failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        ui->stackedWidget->setCurrentWidget(ui->mainPage);
    });
}

void OnlineTerms::on_logoutButton_clicked()
{
    OnlineApi::instance()->logOut();
    ui->backButton->click();
}

void OnlineTerms::on_viewPrivacyPolicyButton_clicked()
{
    view(tr("Privacy Policy"), "privacy");
}

void OnlineTerms::on_viewPageTextBrowser_anchorClicked(const QUrl &arg1)
{
    QDesktopServices::openUrl(arg1);
}
