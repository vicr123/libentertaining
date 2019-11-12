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
#include "friendsdialog.h"
#include "ui_friendsdialog.h"

#include <the-libs_global.h>
#include "private/friendsmodel.h"
#include "pauseoverlay.h"
#include "textinputoverlay.h"
#include "questionoverlay.h"
#include "onlineapi.h"
#include <QKeyEvent>

struct FriendsDialogPrivate {
    FriendsModel* model;
};

FriendsDialog::FriendsDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendsDialog)
{
    ui->setupUi(this);
    d = new FriendsDialogPrivate();

    d->model = new FriendsModel();

    ui->leftWidget->setFixedWidth(SC_DPI(300));
    ui->friendsList->setModel(d->model);
    ui->friendsList->setItemDelegate(new FriendsDelegate);
    ui->friendsList->installEventFilter(this);

    ui->logOutButton->setProperty("type", "destructive");

    ui->profilePage->setFocusProxy(ui->changeUsernameButton);
    ui->addFriendPage->setFocusProxy(ui->addFriendByUsernameButton);
    this->setFocusProxy(ui->friendsList);

    connect(ui->friendsList->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](QItemSelection current, QItemSelection previous) {
        QModelIndex index = current.indexes().first();
        QString pane = index.data(Qt::UserRole).toString();
        if (pane == "profile") {
            ui->stackedWidget->setCurrentWidget(ui->profilePage);
        } else if (pane == "friend-add") {
            ui->stackedWidget->setCurrentWidget(ui->addFriendPage);
        } else if (pane == "friend") {
            ui->friendPage->setActiveUser(index.data(Qt::DisplayRole).toString(), index.data(Qt::UserRole + 2).toString());
            ui->stackedWidget->setCurrentWidget(ui->friendPage);
        }
    });

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        if (ui->friendsList->hasFocus()) {
            ui->backButton->click();
        } else {
            ui->friendsList->setFocus();
        }
    });

    ui->focusBarrier->setBounceWidget(ui->addFriendByUsernameButton);
    ui->focusBarrier_2->setBounceWidget(ui->addFriendByUsernameButton);
    ui->focusBarrier_3->setBounceWidget(ui->changeUsernameButton);
    ui->focusBarrier_4->setBounceWidget(ui->logOutButton);

    ui->profilePageTitle->setText(OnlineApi::getLoggedInUsername());

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);
}

FriendsDialog::~FriendsDialog()
{
    delete ui;
//    delete d;
}

void FriendsDialog::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit done();
    });
}

void FriendsDialog::on_addFriendByUsernameButton_clicked()
{
    bool canceled;
    QString username = TextInputOverlay::getText(this, tr("What's your friend's username?"), &canceled);
    if (!canceled) {
        //Send the friend request
        OnlineApi::instance()->post("/friends/requestByUsername", {
            {"username", username}
        })->then([=](QJsonDocument doc) {
            QuestionOverlay* question = new QuestionOverlay(this);
            if (doc.object().contains("error")) {
                question->setIcon(QMessageBox::Critical);
                QString error = doc.object().value("error").toString();
                if (error == "user.unknownTarget") {
                    question->setTitle(tr("Unknown User"));
                    question->setText(tr("That user doesn't exist."));
                } else if (error == "friends.alreadyFriends") {
                    question->setTitle(tr("Already Friends"));
                    question->setText(tr("You're already friends with that user."));
                } else {
                    question->setTitle(tr("Friend Request Failed"));
                    question->setText(error);
                }
            } else {
                question->setIcon(QMessageBox::Information);
                question->setTitle(tr("Friend Request Sent"));
                question->setText(tr("Your friend request to %1 has been sent.").arg(username));
            }
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        })->error([=](QString error) {
            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Friend Request Failed"));
            question->setText(OnlineApi::errorFromPromiseRejection(error));
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        });
    }
}

bool FriendsDialog::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == ui->friendsList && event->type() == QEvent::KeyPress) {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space) {
            ui->stackedWidget->currentWidget()->setFocus();
        } else if (e->key() == Qt::Key_Up) {
            if (ui->friendsList->model()->index(ui->friendsList->currentIndex().row() - 1, 0).data(Qt::UserRole).toString() == "sep") {
                ui->friendsList->setCurrentIndex(ui->friendsList->model()->index(ui->friendsList->currentIndex().row() - 2, 0));
                return true;
            }
        } else if (e->key() == Qt::Key_Down) {
            if (ui->friendsList->model()->index(ui->friendsList->currentIndex().row() + 1, 0).data(Qt::UserRole).toString() == "sep") {
                ui->friendsList->setCurrentIndex(ui->friendsList->model()->index(ui->friendsList->currentIndex().row() + 2, 0));
                return true;
            }
        }
    }
    return false;
}
