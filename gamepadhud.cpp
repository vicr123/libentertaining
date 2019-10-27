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

#include <QToolButton>
#include "gamepadevent.h"
#include "gamepadbuttons.h"

struct GamepadHudPrivate {
    QMap<QGamepadManager::GamepadButton, QToolButton*> hudItems;
    QMap<QGamepadManager::GamepadButton, std::function<void()>> buttonActions;

    QWidget* parent = nullptr;
};

GamepadHud::GamepadHud(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GamepadHud)
{
    ui->setupUi(this);
    d = new GamepadHudPrivate();

    this->setParent(parent);
}

GamepadHud::~GamepadHud()
{
    delete ui;
    delete d;
}

void GamepadHud::setParent(QWidget* parent)
{
    if (d->parent != nullptr) d->parent->removeEventFilter(this);
    QWidget::setParent(parent);
    d->parent = parent;
    if (parent != nullptr) parent->installEventFilter(this);
}

void GamepadHud::addListener(QWidget*listenTo)
{
    listenTo->installEventFilter(this);
}

void GamepadHud::removeListener(QWidget*listenTo)
{
    listenTo->removeEventFilter(this);
}

void GamepadHud::setButtonText(QGamepadManager::GamepadButton button, QString text)
{
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
        item->setIcon(GamepadButtons::iconForButton(button, this->palette().color(QPalette::WindowText)));
        connect(item, &QToolButton::clicked, this, [=] {
            if (d->buttonActions.contains(button)) {
                d->buttonActions.value(button)();
            }
        });

        ui->buttonLayout->addWidget(item);
        d->hudItems.insert(button, item);
    }

    item->setText(text);
}

void GamepadHud::setButtonAction(QGamepadManager::GamepadButton button, std::function<void ()> action)
{
    d->buttonActions.insert(button, action);
}

void GamepadHud::removeText(QGamepadManager::GamepadButton button)
{
    if (d->hudItems.contains(button)) {
        QToolButton* item = d->hudItems.value(button);
        d->hudItems.remove(button);
        ui->buttonLayout->removeWidget(item);
        item->deleteLater();
    }
}

void GamepadHud::removeButtonAction(QGamepadManager::GamepadButton button)
{
    d->buttonActions.remove(button);
}

bool GamepadHud::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == d->parent && event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->isButtonEvent() && e->buttonPressed() && d->buttonActions.contains(e->button())) {
            //Run the button action
            d->buttonActions.value(e->button())();

            //Prevent propagation
            e->accept();
            return true;
        }
    }
    return false;
}

