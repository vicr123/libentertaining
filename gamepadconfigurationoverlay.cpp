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
#include "gamepadconfigurationoverlay.h"
#include "ui_gamepadconfigurationoverlay.h"

#include "pauseoverlay.h"
#include <the-libs_global.h>
#include "private/gamepadmodel.h"
#include <QShortcut>
#include <QKeyEvent>
#include "gamepadConfiguration/buttondiagnostics.h"
#include "gamepadConfiguration/stickdiagnostics.h"

GamepadConfigurationOverlay::GamepadConfigurationOverlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GamepadConfigurationOverlay)
{
    ui->setupUi(this);

    ui->gamepadsWidget->setFixedWidth(SC_DPI(300));
    ui->gamepadsWidget->setModel(new GamepadModel(this));
    connect(ui->gamepadsWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](QItemSelection current, QItemSelection previous) {
        if (current.indexes().count() == 0) {
            ui->stackedWidget->setCurrentWidget(ui->selectGamepadPage);
        } else {
            ui->stackedWidget->setCurrentWidget(ui->gamepadConfigurationPage);
            ui->gamepadIdLabel->setText(tr("Gamepad ID: %1").arg(current.indexes().first().data(Qt::UserRole).toInt()));
        }
    });
    if (ui->gamepadsWidget->model()->rowCount() > 0) ui->gamepadsWidget->selectionModel()->select(ui->gamepadsWidget->model()->index(0, 0), QItemSelectionModel::SelectCurrent);
    ui->gamepadsWidget->installEventFilter(this);

    ui->gamepadConfigurationPage->setFocusProxy(ui->configureButtonMappings);

    this->setFocusProxy(ui->gamepadsWidget);

    ui->focusBarrierTop->setBounceWidget(ui->configureButtonMappings);
    ui->focusBarrierBottom->setBounceWidget(ui->checkSticks);

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        if (ui->gamepadsWidget->hasFocus()) {
            ui->backButton->click();
        } else {
            ui->gamepadsWidget->setFocus();
        }
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        ui->backButton->click();
    });
}

GamepadConfigurationOverlay::~GamepadConfigurationOverlay()
{
    delete ui;
}

void GamepadConfigurationOverlay::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit done();
    });
}

bool GamepadConfigurationOverlay::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == ui->gamepadsWidget && event->type() == QEvent::KeyPress) {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space) {
            if (ui->stackedWidget->currentWidget() == ui->gamepadConfigurationPage) {
                ui->gamepadConfigurationPage->setFocus();
            }
        }
    }
    return false;
}

void GamepadConfigurationOverlay::on_checkButtons_clicked()
{
    ButtonDiagnostics* diag = new ButtonDiagnostics(ui->gamepadsWidget->currentIndex().data(Qt::UserRole).toInt(), this);
    connect(diag, &ButtonDiagnostics::done, diag, &ButtonDiagnostics::deleteLater);
}

void GamepadConfigurationOverlay::on_checkSticks_clicked()
{
    StickDiagnostics* diag = new StickDiagnostics(ui->gamepadsWidget->currentIndex().data(Qt::UserRole).toInt(), this);
    connect(diag, &StickDiagnostics::done, diag, &StickDiagnostics::deleteLater);
}
