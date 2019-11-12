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
#include "friendpage.h"
#include "ui_friendpage.h"

#include "questionoverlay.h"
#include "online/onlineapi.h"
#include "questionoverlay.h"

struct FriendPagePrivate {
    QString currentUsername;
};

FriendPage::FriendPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendPage)
{
    ui->setupUi(this);
    d = new FriendPagePrivate();

    ui->friendPage->setFocusProxy(ui->removeFriendButton);
    ui->incomingRequestPage->setFocusProxy(ui->acceptIncomingButton);
    ui->outgoingRequestPage->setFocusProxy(ui->retractRequestButton);

    ui->focusBarrier->setBounceWidget(ui->acceptIncomingButton);
    ui->focusBarrier_2->setBounceWidget(ui->blockIncomingButton);
    ui->focusBarrier_3->setBounceWidget(ui->removeFriendButton);
    ui->focusBarrier_4->setBounceWidget(ui->blockButton);
    ui->focusBarrier_5->setBounceWidget(ui->retractRequestButton);
    ui->focusBarrier_6->setBounceWidget(ui->retractRequestButton);
}

FriendPage::~FriendPage()
{
    delete d;
    delete ui;
}

void FriendPage::setActiveUser(QString username, QString friendStatus)
{
    ui->titleLabel->setText(username);
    d->currentUsername = username;
    if (friendStatus == "friend") {
        ui->stackedWidget->setCurrentWidget(ui->friendPage);
    } else if (friendStatus == "request-incoming") {
        ui->stackedWidget->setCurrentWidget(ui->incomingRequestPage);
    } else if (friendStatus == "request-outgoing") {
        ui->outgoingRequestExplanation->setText(tr("We're awaiting %1's response.").arg(username));
        ui->stackedWidget->setCurrentWidget(ui->outgoingRequestPage);
    }
    this->setFocusProxy(ui->stackedWidget->currentWidget());
}

void FriendPage::on_acceptIncomingButton_clicked()
{
    respondToFriendRequest("accept");
}

void FriendPage::on_declineIncomingButton_clicked()
{
    respondToFriendRequest("decline");
}

void FriendPage::respondToFriendRequest(QString response)
{
    //Accept the friend request
    QString endpoint;
    if (response == "accept") {
        endpoint = "/friends/acceptByUsername";
    } else if (response == "decline") {
        endpoint = "/friends/declineByUsername";
    } else if (response == "retract") {
        endpoint = "/friends/retractByUsername";
    }

    emit blockUi(true);
    OnlineApi::instance()->post(endpoint, {
        {"username", d->currentUsername}
    })->then([=](QJsonDocument doc) {
        QuestionOverlay* question = new QuestionOverlay(this);
        if (doc.object().contains("error")) {
            question->setIcon(QMessageBox::Critical);
            QString error = doc.object().value("error").toString();
            if (error == "user.unknownTarget") {
                question->setTitle(tr("Unknown User"));
                question->setText(tr("That user doesn't exist."));
            } else {
                question->setTitle(tr("Friend Request Failed"));
                question->setText(error);
            }
        } else {
            question->setIcon(QMessageBox::Information);
            if (response == "accept") {
                question->setTitle(tr("Friend Request Accepted"));
                question->setText(tr("The friend request from %1 has been accepted.").arg(d->currentUsername));

                setActiveUser(d->currentUsername, "friend");
            } else if (response == "decline") {
                question->setTitle(tr("Friend Request Declined"));
                question->setText(tr("The friend request from %1 has been removed.").arg(d->currentUsername));
            } else if (response == "retract") {
                question->setTitle(tr("Friend Request Retracted"));
                question->setText(tr("Your friend request to %1 has been retracted.").arg(d->currentUsername));
            }
        }
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

        emit blockUi(false);
        emit reloadFriendsModel();
    })->error([=](QString error) {
        QuestionOverlay* question = new QuestionOverlay(this);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Friend Request Failed"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);
        connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
        connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        emit blockUi(false);
    });
}

void FriendPage::on_removeFriendButton_clicked()
{
    QuestionOverlay* question = new QuestionOverlay(this);
    question->setIcon(QMessageBox::Question);
    question->setTitle(tr("Remove Friend"));
    question->setText(tr("Remove %1 as a friend?").arg(d->currentUsername));
    question->setButtons(QMessageBox::Yes | QMessageBox::No, tr("Remove"));
    connect(question, &QuestionOverlay::accepted, this, [=](QMessageBox::StandardButton button) {
        if (button == QMessageBox::Yes) {
            //Remove friend
            emit blockUi(true);
            OnlineApi::instance()->post("/friends/removeByUsername", {
                {"username", d->currentUsername}
            })->then([=](QJsonDocument doc) {
                QuestionOverlay* question = new QuestionOverlay(this);
                if (doc.object().contains("error")) {
                    question->setIcon(QMessageBox::Critical);
                    QString error = doc.object().value("error").toString();
                    if (error == "user.unknownTarget") {
                        question->setTitle(tr("Unknown User"));
                        question->setText(tr("That user doesn't exist."));
                    } else if (error == "friends.notFriends") {
                        question->setTitle(tr("Not Friends"));
                        question->setText(tr("You're not friends with this user."));
                    } else {
                        question->setTitle(tr("Friend Removal Failed"));
                        question->setText(error);
                    }
                } else {
                    question->setIcon(QMessageBox::Information);
                    question->setTitle(tr("Friend Removed"));
                    question->setText(tr("%1 has been removed from your friend list.").arg(d->currentUsername));
                }
                question->setButtons(QMessageBox::Ok);
                connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

                emit reloadFriendsModel();
                emit blockUi(false);
            })->error([=](QString error) {
                QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Friend Removal Failed"));
                question->setText(OnlineApi::errorFromPromiseRejection(error));
                question->setButtons(QMessageBox::Ok);
                connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
                emit blockUi(false);
            });
        }
    });
    connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
}

void FriendPage::on_retractRequestButton_clicked()
{
    respondToFriendRequest("retract");
}
