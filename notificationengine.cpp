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
#include "notificationengine.h"

#include <QWidget>
#include <QEvent>
#include <QBoxLayout>
#include <QShortcut>
#include <QQueue>
#include <QTimer>
#include "gamepadlabel.h"
#include "gamepadbuttons.h"
#include "focusbarrier.h"
#include "gamepadevent.h"
#include <the-libs_global.h>
#include "notifications/notificationpopup.h"

struct NotificationEnginePrivate {
    NotificationEngine* instance = nullptr;
    QWidget* window = nullptr;

    QWidget* container;
    QBoxLayout* notificationsLayout;

    quint64 current = 0;
    QMap<quint64, NotificationPopup*> popups;
    QQueue<NotificationPopup*> showingPopups;

    GamepadLabel* focusPrompt;

    FocusBarrier* bar1;
    FocusBarrier* bar2;

    QShortcut* shortcut;
};

NotificationEnginePrivate* NotificationEngine::d = new NotificationEnginePrivate();

void NotificationEngine::setApplicationNotificationWindow(QWidget*window)
{
    instance();

    if (d->window) {
        d->window->removeEventFilter(d->instance);
        d->shortcut->deleteLater();
    }

    d->window = window;
    d->window->installEventFilter(d->instance);
    d->container->setParent(d->window);
    d->container->move(d->window->width() - d->container->width(), 0);
    d->container->show();

    d->shortcut = new QShortcut(QKeySequence(Qt::Key_F12), window);
    connect(d->shortcut, &QShortcut::activated, d->instance, &NotificationEngine::focusNotifications);
}

quint64 NotificationEngine::push(NotificationData notification)
{
    instance();
    if (d->window == nullptr) return 0;
    if (QApplication::closingDown()) return 0;

    quint64 notificationNumber = d->current++;

    NotificationPopup* popup = new NotificationPopup(notification);
    connect(popup, &NotificationPopup::dismiss, d->instance, [=] {
        removePopup(popup);
    });
    connect(popup, &NotificationPopup::activatedAction, d->instance, [=](QString key) {
        emit d->instance->actionClicked(notificationNumber, key);
        removePopup(popup);
    });
    connect(popup, &NotificationPopup::destroyed, d->instance, [=] {
        d->popups.remove(notificationNumber);
    });

    d->notificationsLayout->addWidget(popup);
    d->showingPopups.enqueue(popup);

    d->instance->setFirstPopup();

    int dismissAfter = notification.dismissTimer;
    if (dismissAfter == -1) dismissAfter = 5000;
    if (dismissAfter != 0) {
        QTimer::singleShot(dismissAfter, [=] {
            removePopup(popup);
        });
    }

    return notificationNumber;
}

void NotificationEngine::dismiss(quint64 id)
{
    if (d->popups.contains(id)) {
        NotificationPopup* popup = d->popups.value(id);
        removePopup(popup);
    }
}

void NotificationEngine::focusNotifications()
{
    if (d->showingPopups.count() > 0) {
        d->showingPopups.first()->setFocus();
    }
}

NotificationEngine*NotificationEngine::instance()
{
    if (d->instance == nullptr) d->instance = new NotificationEngine();
    return d->instance;
}

NotificationEngine::NotificationEngine() : QObject(nullptr)
{
    d->container = new QWidget();
    d->container->setFocusPolicy(Qt::NoFocus);
    d->container->installEventFilter(this);

    QBoxLayout* mainContainerLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    mainContainerLayout->setSizeConstraint(QBoxLayout::SetFixedSize);
    d->container->setLayout(mainContainerLayout);

    d->bar1 = new FocusBarrier();
    mainContainerLayout->addWidget(d->bar1);
    d->bar1->setVisible(true);

    d->notificationsLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    mainContainerLayout->addLayout(d->notificationsLayout);

    QBoxLayout* promptLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    promptLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
    d->focusPrompt = new GamepadLabel();
    d->focusPrompt->setText(tr("%1 / %2 to focus").arg(GamepadButtons::stringForKey(Qt::Key_F12)).arg(GamepadButtons::stringForButton(QGamepadManager::ButtonSelect)));
    d->focusPrompt->setVisible(false);
    promptLayout->addWidget(d->focusPrompt);

    d->notificationsLayout->addLayout(promptLayout);

    d->bar2 = new FocusBarrier();
    mainContainerLayout->addWidget(d->bar2);
    d->bar2->setVisible(true);
}

bool NotificationEngine::eventFilter(QObject*watched, QEvent*event)
{
    if (event->type() == QEvent::Resize) {
        d->container->move(d->window->width() - d->container->width(), 0);
    } else if (watched == d->window && event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->isButtonEvent() && e->buttonPressed() && e->button() == QGamepadManager::ButtonSelect) {
            e->setAccepted(true);
            focusNotifications();
            return true;
        }
    }
    return false;
}

void NotificationEngine::removePopup(NotificationPopup*popup)
{
    if (d->showingPopups.contains(popup)) {
        d->showingPopups.removeAll(popup);
        d->notificationsLayout->removeWidget(popup);
        popup->setVisible(false);
        d->instance->setFirstPopup();
    }
}

void NotificationEngine::setFirstPopup()
{
    if (d->showingPopups.count() > 0) {
//        NotificationPopup* popup = d->showingPopups.first();
        d->focusPrompt->setVisible(true);

        QWidget* before = d->bar1;
        for (QWidget* w : d->showingPopups) {
            QWidget::setTabOrder(before, w);
            before = w;
        }
        QWidget::setTabOrder(before, d->bar2);

        d->bar1->setBounceWidget(d->showingPopups.last());
        d->bar2->setBounceWidget(d->showingPopups.first());
    } else {
        d->focusPrompt->setVisible(false);
    }
}

NotificationData::NotificationData()
{

}

NotificationData::NotificationData(QString title, QString text, QIcon icon, QMap<QString, QString> actions)
{
    this->title = title;
    this->text = text;
    this->icon = icon;
    this->actions = actions;
}
