/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#include "buttonmappingitem.h"
#include "ui_buttonmappingitem.h"

#include "gamepadbuttons.h"

struct ButtonMappingItemPrivate {
    QGamepadManager::GamepadButton button;
    int gamepadId;
};

ButtonMappingItem::ButtonMappingItem(QGamepadManager::GamepadButton button, int gamepadId, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ButtonMappingItem) {
    ui->setupUi(this);

    d = new ButtonMappingItemPrivate();
    d->button = button;
    d->gamepadId = gamepadId;

    ui->buttonLabel->setText(GamepadButtons::stringForButton(button));
    this->setFocusPolicy(Qt::StrongFocus);

    connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured, this, [ = ](int deviceId, QGamepadManager::GamepadButton button) {
        if (deviceId == gamepadId && button == d->button) ui->mapButton->setChecked(false);
    });
    connect(QGamepadManager::instance(), &QGamepadManager::configurationCanceled, this, [ = ](int deviceId) {
        if (deviceId == gamepadId) ui->mapButton->setChecked(false);
    });
}

ButtonMappingItem::~ButtonMappingItem() {
    delete ui;
    delete d;
}

#include <QDebug>
void ButtonMappingItem::on_mapButton_toggled(bool checked) {
    if (checked) {
        bool success = QGamepadManager::instance()->configureButton(d->gamepadId, d->button);
        qDebug() << success;
    }
}
