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
#include "notificationpopup.h"
#include "ui_notificationpopup.h"

#include "notificationengine.h"
#include <QIcon>
#include <QPushButton>
#include <QKeyEvent>
#include "gamepadevent.h"
#include <the-libs_global.h>

NotificationPopup::NotificationPopup(NotificationData notification, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationPopup)
{
    ui->setupUi(this);

    ui->titleLabel->setText(notification.title);
    ui->textLabel->setText(notification.text);

    if (notification.icon.isNull()) {
        ui->icon->setVisible(false);
    } else {
        ui->icon->setPixmap(notification.icon.pixmap(SC_DPI_T(QSize(32, 32), QSize)));
    }

    QPushButton* firstButton = nullptr;
    if (notification.actions.count() > 0) {
        for (auto i = notification.actions.begin(); i != notification.actions.end(); i++) {
            QPushButton* button = new QPushButton(this);
            button->setText(i.value());

            QString action = i.key();
            connect(button, &QPushButton::clicked, this, [=] {
                emit activatedAction(action);
                emit dismiss();
            });
            ui->buttonContainer->layout()->addWidget(button);

            if (firstButton == nullptr) firstButton = button;
        }
    } else {
        ui->buttonContainer->setVisible(false);
    }

    this->setFixedWidth(SC_DPI(400));

    if (!notification.dismissable && firstButton) {
        ui->dismissButton->setVisible(false);
        this->setFocusProxy(firstButton);
    } else {
        this->setFocusProxy(ui->dismissButton);
    }
}

NotificationPopup::~NotificationPopup()
{
    delete ui;
}

void NotificationPopup::on_dismissButton_clicked()
{
    emit dismiss();
}

bool NotificationPopup::event(QEvent *event) {
    if (event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->isButtonEvent() && e->buttonPressed() && e->button() == QGamepadManager::ButtonA) {
            e->setAccepted(true);

            QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
            QApplication::sendEvent(QApplication::focusWidget(), &event);

            QKeyEvent event2(QKeyEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
            QApplication::sendEvent(QApplication::focusWidget(), &event2);

            return true;
        }
    }
    return QWidget::event(event);
}
