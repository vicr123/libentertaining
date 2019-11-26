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
    ui->stackedWidget->setCurrentWidget(ui->communityGuidelinesPage);
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

    QNetworkReply* cgReply = d->mgr.get(QNetworkRequest(OnlineApi::instance()->urlForPath("/info/communityguidelines")));
    connect(cgReply, &QNetworkReply::finished, this, [=] {
        if (cgReply->error() == QNetworkReply::NoError) {
            ui->communityGuidelinesTextBrowser->setHtml(cgReply->readAll());
        } else {
            ui->communityGuidelinesTextBrowser->setText(tr("Encountered an error trying to load the community guidelines. Read them online at <a href=\"%1\">%1</a>")
                                                        .arg(OnlineApi::instance()->urlForPath("/info/communityguidelines").toString()));
        }
    });

    if (d->isTermsChanged) {
        ui->descriptionLabel->setText(tr("The Terms and Conditions or Community Guidelines have been updated. To continue playing online, you'll need to read and agree to the new documents."));
        ui->focusBarrier_2->setBounceWidget(ui->logoutButton);
    } else {
        ui->descriptionLabel->setText(tr("The Terms and Conditions and Community Guidelines govern your use of the Entertaining Games services. By creating an account and using the services, you agree to these documents."));
        ui->acceptButton->setVisible(false);
        ui->logoutButton->setVisible(false);
        ui->focusBarrier_2->setBounceWidget(ui->viewCommunityGuidelinesButton);
    }

    this->setFocusProxy(ui->viewCommunityGuidelinesButton);
    ui->mainPage->setFocusProxy(ui->viewCommunityGuidelinesButton);
    ui->communityGuidelinesPage->setFocusProxy(ui->communityGuidelinesTextBrowser);
    ui->logoutButton->setProperty("type", "destructive");

    ui->focusBarrier->setBounceWidget(ui->viewCommunityGuidelinesButton);
    ui->focusBarrier_3->setBounceWidget(ui->communityGuidelinesTextBrowser);
    ui->focusBarrier_4->setBounceWidget(ui->communityGuidelinesTextBrowser);

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

void OnlineTerms::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit rejected();
    });
}

void OnlineTerms::on_acceptButton_clicked()
{
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
    });
}

void OnlineTerms::on_logoutButton_clicked()
{
    OnlineApi::instance()->logOut();
    ui->backButton->click();
}
