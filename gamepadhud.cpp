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

#include <QPushButton>
#include "gamepadbuttons.h"

struct GamepadHudPrivate {
    QMap<QGamepadManager::GamepadButton, QPushButton*> hudItems;
    QMap<QGamepadManager::GamepadButton, std::function<void()>> buttonActions;
};

GamepadHud::GamepadHud(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GamepadHud)
{
    ui->setupUi(this);
    d = new GamepadHudPrivate();
}

GamepadHud::~GamepadHud()
{
    delete ui;
    delete d;
}

void GamepadHud::setButtonText(QGamepadManager::GamepadButton button, QString text)
{
//    d->buttonText.insert(button, text);
    QPushButton* item;
    if (d->hudItems.contains(button)) {
        item = d->hudItems.value(button);
    } else {
        item = new QPushButton();

        QPalette pal = item->palette();
        pal.setColor(QPalette::Window, Qt::transparent);
        item->setPalette(pal);

        item->setFlat(true);
        item->setFocusPolicy(Qt::NoFocus);
        item->setIcon(GamepadButtons::iconForButton(button, this->palette().color(QPalette::WindowText)));
        connect(item, &QPushButton::clicked, this, [=] {
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
        QPushButton* item = d->hudItems.value(button);
        d->hudItems.remove(button);
        ui->buttonLayout->removeWidget(item);
        item->deleteLater();
    }
}

