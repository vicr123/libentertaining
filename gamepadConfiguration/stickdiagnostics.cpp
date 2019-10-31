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
#include "stickdiagnostics.h"
#include "ui_stickdiagnostics.h"

#include <QGamepad>
#include "gamepadevent.h"
#include "pauseoverlay.h"
#include "questionoverlay.h"

struct StickDiagnosticsPrivate {
    QGamepad gamepad;
};

StickDiagnostics::StickDiagnostics(int gamepadId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StickDiagnostics)
{
    ui->setupUi(this);
    d = new StickDiagnosticsPrivate();
    d->gamepad.setDeviceId(gamepadId);

    ui->stickL->setSide(tr("L", "L for (L)eft"));
    ui->stickR->setSide(tr("R", "R for (R)ight"));

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);

    connect(&d->gamepad, &QGamepad::connectedChanged, this, [=](bool connected) {
        if (!connected) {
            d->gamepad.blockSignals(true);

            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Gamepad Disconnected"));
            question->setText(tr("The gamepad that you were testing was disconnected."));
            question->setButtons(QMessageBox::Ok);

            auto after = [=] {
                question->deleteLater();
                quit();
            };
            connect(question, &QuestionOverlay::accepted, this, after);
            connect(question, &QuestionOverlay::rejected, this, after);
        }
    });

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Done"));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->finishButton->click();
    });
}

StickDiagnostics::~StickDiagnostics()
{
    delete ui;
    delete d;
}

void StickDiagnostics::quit()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit done();
    });
}

bool StickDiagnostics::event(QEvent*event)
{
    if (event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->gamepad()->deviceId() == d->gamepad.deviceId() && !e->isButtonEvent()) {
            StickDiagnosticsStickWidget* w;
            if (e->axis() == QGamepadManager::AxisLeftX || e->axis() == QGamepadManager::AxisLeftY) {
                w = ui->stickL;
            } else {
                w = ui->stickR;
            }

            if (e->axis() == QGamepadManager::AxisLeftX || e->axis() == QGamepadManager::AxisRightX) {
                w->setXAxis(e->newValue());
            } else {
                w->setYAxis(e->newValue());
            }
        }
        e->accept();
        return true;
    }
    return false;
}

void StickDiagnostics::on_finishButton_clicked()
{
    quit();
}
