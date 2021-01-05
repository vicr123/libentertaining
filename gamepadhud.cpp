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
#include "gamepadhud.h"
#include "ui_gamepadhud.h"

#include <QKeyEvent>
#include <QToolButton>
#include "gamepadevent.h"
#include "gamepadbuttons.h"
#include "private/entertainingsettings.h"

struct GamepadHudPrivate {
    QMap<QGamepadManager::GamepadButton, QToolButton*> hudItems;
    QMap<QGamepadManager::GamepadButton, std::function<void()>> buttonActions;
    QMap<QKeySequence, QGamepadManager::GamepadButton> keyBinds = {
        {Qt::Key_Return, QGamepadManager::ButtonA},
        {Qt::Key_Escape, QGamepadManager::ButtonB}
    };

    QWidget* parent = nullptr;

    bool isShowingGamepadButtons = false;
};

GamepadHud::GamepadHud(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GamepadHud) {
    ui->setupUi(this);
    d = new GamepadHudPrivate();

    this->setParent(parent);
    connect(static_cast<QApplication*>(QApplication::instance()), &QApplication::focusChanged, this, [ = ](QWidget * oldFocus, QWidget * newFocus) {
        if (oldFocus) oldFocus->removeEventFilter(this);
        if (newFocus) newFocus->installEventFilter(this);
    });

    connect(EntertainingSettings::instance(), &tSettings::settingChanged, this, [ = ](QString key, QVariant value) {
        if (key == "gamepad/icons") this->setShowGamepadButtons(d->isShowingGamepadButtons);
    });
}

GamepadHud::~GamepadHud() {
    delete ui;
    delete d;
}

void GamepadHud::setParent(QWidget* parent) {
    if (d->parent != nullptr) d->parent->removeEventFilter(this);
    QWidget::setParent(parent);
    d->parent = parent;
    if (parent != nullptr) parent->installEventFilter(this);
}

void GamepadHud::addListener(QWidget* listenTo) {
    listenTo->installEventFilter(this);
}

void GamepadHud::removeListener(QWidget* listenTo) {
    listenTo->removeEventFilter(this);
}

void GamepadHud::setButtonText(QGamepadManager::GamepadButton button, QString text) {
//    d->buttonText.insert(button, text);
    QToolButton* item;
    if (d->hudItems.contains(button)) {
        item = d->hudItems.value(button);
    } else {
        item = new QToolButton();

        QPalette pal = item->palette();
        pal.setColor(QPalette::Window, Qt::transparent);
        item->setPalette(pal);

        item->setAutoRaise(true);
        item->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        item->setFocusPolicy(Qt::NoFocus);
        if (d->isShowingGamepadButtons || !d->keyBinds.values().contains(button)) {
            item->setIcon(GamepadButtons::iconForButton(button, this->palette().color(QPalette::WindowText)));
        } else {
            item->setIcon(GamepadButtons::iconForKey(d->keyBinds.key(button), this->font(), this->palette()));
        }

        connect(item, &QToolButton::clicked, this, [ = ] {
            if (d->buttonActions.contains(button)) {
                d->buttonActions.value(button)();
            }
        });

        ui->buttonLayout->addWidget(item);
        d->hudItems.insert(button, item);
    }

    item->setText(text);
}

void GamepadHud::setButtonAction(QGamepadManager::GamepadButton button, std::function<void ()> action) {
    d->buttonActions.insert(button, action);
}

void GamepadHud::removeText(QGamepadManager::GamepadButton button) {
    if (d->hudItems.contains(button)) {
        QToolButton* item = d->hudItems.value(button);
        d->hudItems.remove(button);
        ui->buttonLayout->removeWidget(item);
        item->deleteLater();
    }
}

void GamepadHud::removeButtonAction(QGamepadManager::GamepadButton button) {
    d->buttonActions.remove(button);
}

void GamepadHud::bindKey(QKeySequence key, QGamepadManager::GamepadButton button) {
    d->keyBinds.insert(key, button);
    if (!d->isShowingGamepadButtons) {
        QToolButton* item = d->hudItems.value(button);
        if (item) item->setIcon(GamepadButtons::iconForKey(key, this->font(), this->palette()));
    }
}

void GamepadHud::unbindKey(QKeySequence key) {
    QToolButton* item = d->hudItems.value(d->keyBinds.value(key));
    if (item) item->setIcon(GamepadButtons::iconForButton(d->keyBinds.value(key), this->palette().color(QPalette::WindowText)));
    d->keyBinds.remove(key);
}

std::function<void ()> GamepadHud::standardAction(GamepadHud::StandardAction action) {
    switch (action) {
        case GamepadHud::SelectAction:
            return [ = ] {
                QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
                QApplication::sendEvent(QApplication::focusWidget(), &event);

                QKeyEvent event2(QKeyEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
                QApplication::sendEvent(QApplication::focusWidget(), &event2);
            };
    }
}

bool GamepadHud::eventFilter(QObject* watched, QEvent* event) {
    if (watched == QApplication::focusWidget()) {
        if (event->spontaneous() && event->type() == QEvent::KeyPress && d->isShowingGamepadButtons) {
            setShowGamepadButtons(false);
        } else if (event->type() == GamepadEvent::type() && !d->isShowingGamepadButtons) {
            setShowGamepadButtons(true);
        }
    } else if (watched == d->parent && event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->isButtonEvent() && e->buttonPressed() && d->buttonActions.contains(e->button())) {
            //Run the button action
            d->buttonActions.value(e->button())();

            //Prevent propagation
            e->accept();
            return true;
        }
    } else if (watched == d->parent && event->type() == QEvent::KeyPress) {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        Qt::Key key = static_cast<Qt::Key>(e->key() | e->modifiers());
        QKeySequence keySeq(key);

        if (d->keyBinds.contains(keySeq) && d->buttonActions.contains(d->keyBinds.value(keySeq))) {
            //Run the button action
            d->buttonActions.value(d->keyBinds.value(keySeq))();

            //Prevent propagation
            e->accept();
            return true;
        }
    } else if (watched == d->parent && event->type() == QEvent::KeyRelease) {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
#ifdef Q_OS_ANDROID
        if (e->key() == Qt::Key_Back) {
            if (d->buttonActions.contains(QGamepadManager::ButtonB)) {
                //Run the back action
                d->buttonActions.value(QGamepadManager::ButtonB)();
            } else if (d->buttonActions.contains(QGamepadManager::ButtonStart)) {
                //Run the pause action
                d->buttonActions.value(QGamepadManager::ButtonStart)();
            }
            event->accept();
            return true;
        }
#endif
    }
    return false;
}

void GamepadHud::setShowGamepadButtons(bool showGamepadButtons) {
    d->isShowingGamepadButtons = showGamepadButtons;
    for (QGamepadManager::GamepadButton btn : d->hudItems.keys()) {
        QToolButton* button = d->hudItems.value(btn);
        if (showGamepadButtons || !d->keyBinds.values().contains(btn)) {
            button->setIcon(GamepadButtons::iconForButton(btn, this->palette().color(QPalette::WindowText)));
        } else {
            button->setIcon(GamepadButtons::iconForKey(d->keyBinds.key(btn), this->font(), this->palette()));
        }
    }
}

