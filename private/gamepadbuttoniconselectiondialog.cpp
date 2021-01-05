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
#include "gamepadbuttoniconselectiondialog.h"
#include "ui_gamepadbuttoniconselectiondialog.h"

#include <QShortcut>
#include "gamepadbuttons.h"
#include "entertainingsettings.h"

GamepadButtonIconSelectionDialog::GamepadButtonIconSelectionDialog(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GamepadButtonIconSelectionDialog) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);

    ui->optionsWidget->setFixedWidth(SC_DPI(600));
    this->setFocusProxy(ui->optionsWidget);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [ = ] {
        on_optionsWidget_activated(ui->optionsWidget->currentIndex());
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        emit done();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [ = ] {
        emit done();
    });
    connect(backShortcut, &QShortcut::activatedAmbiguously, this, [ = ] {
        emit done();
    });

    for (int i = 0; i < 4; i++) {
        ui->optionsWidget->addItem(GamepadButtons::iconTypeNameForIndex(i));
    }
}

GamepadButtonIconSelectionDialog::~GamepadButtonIconSelectionDialog() {
    delete ui;
}

void GamepadButtonIconSelectionDialog::on_titleLabel_backButtonClicked() {
    emit done();
}

void GamepadButtonIconSelectionDialog::on_optionsWidget_activated(const QModelIndex& index) {
    EntertainingSettings::instance()->setValue("gamepad/icons", index.row());
    emit done();
}
