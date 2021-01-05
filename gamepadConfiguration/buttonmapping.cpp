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
#include "buttonmapping.h"
#include "ui_buttonmapping.h"

#include "pauseoverlay.h"
#include "questionoverlay.h"
#include "buttonmappingitem.h"
#include <QGamepad>
#include <ttoast.h>

struct ButtonMappingPrivate {
    QGamepad gamepad;
};

ButtonMapping::ButtonMapping(int gamepadId, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ButtonMapping) {
    ui->setupUi(this);

    d = new ButtonMappingPrivate();
    d->gamepad.setDeviceId(gamepadId);

    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->scrollArea->setPalette(pal);
    ui->scrollAreaWidgetContents->setPalette(pal);

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);

    connect(&d->gamepad, &QGamepad::connectedChanged, this, [ = ](bool connected) {
        if (!connected) {
            d->gamepad.blockSignals(true);

            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Gamepad Disconnected"));
            question->setText(tr("The gamepad that you were testing was disconnected."));
            question->setButtons(QMessageBox::Ok);

            auto after = [ = ] {
                question->deleteLater();
                quit();
            };
            connect(question, &QuestionOverlay::accepted, this, after);
            connect(question, &QuestionOverlay::rejected, this, after);
        }
    });

    ui->titleLabel->setBackButtonShown(true);
    ui->mainWidget->setFixedWidth(SC_DPI(600));

    QList<QGamepadManager::GamepadButton> buttons = {
        QGamepadManager::ButtonA,
        QGamepadManager::ButtonB,
        QGamepadManager::ButtonX,
        QGamepadManager::ButtonY,
        QGamepadManager::ButtonL1,
        QGamepadManager::ButtonR1,
        QGamepadManager::ButtonL2,
        QGamepadManager::ButtonR2,
        QGamepadManager::ButtonL3,
        QGamepadManager::ButtonR3,
        QGamepadManager::ButtonUp,
        QGamepadManager::ButtonLeft,
        QGamepadManager::ButtonRight,
        QGamepadManager::ButtonDown,
        QGamepadManager::ButtonStart,
        QGamepadManager::ButtonSelect,
        QGamepadManager::ButtonCenter,
        QGamepadManager::ButtonGuide
    };

    for (QGamepadManager::GamepadButton button : buttons) {
        ButtonMappingItem* item = new ButtonMappingItem(button, gamepadId, this);
        ui->mappingItemsLayout->addWidget(item);
    }
}

ButtonMapping::~ButtonMapping() {
    delete ui;
    delete d;
}

void ButtonMapping::on_titleLabel_backButtonClicked() {
    quit();
}

void ButtonMapping::quit() {
    d->gamepad.blockSignals(true);
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
        emit done();
    });
}

void ButtonMapping::on_resetButton_clicked() {
    QGamepadManager::instance()->resetConfiguration(d->gamepad.deviceId());
    tToast* toast = new tToast();
    toast->setTitle(tr("Gamepad Buttons Reset"));
    toast->setText(tr("Button mappings for this gamepad have been reset."));
    connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
    toast->show(this->window());
}
