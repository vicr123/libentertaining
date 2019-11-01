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
#include "buttondiagnostics.h"
#include "ui_buttondiagnostics.h"

#include "pauseoverlay.h"
#include <QGamepadManager>
#include "gamepadevent.h"
#include "gamepadbuttons.h"
#include <QQueue>
#include <QGamepad>
#include "musicengine.h"
#include <QTimer>
#include "questionoverlay.h"

struct ButtonDiagnosticsPrivate {
    QGamepad gamepad;
    QQueue<QGamepadManager::GamepadButton> pressedOrder;

    QTimer* timer;
    QGamepadManager::GamepadButton timerButton;
};

ButtonDiagnostics::ButtonDiagnostics(int gamepadId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ButtonDiagnostics)
{
    ui->setupUi(this);
    d = new ButtonDiagnosticsPrivate();
    d->gamepad.setDeviceId(gamepadId);

    ui->unavailableButtonsWarning->setText(tr("Depending on your setup, the %1 button may not register here.").arg(GamepadButtons::stringForButton(QGamepadManager::ButtonGuide)));

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

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    d->timer->setInterval(1000);
    connect(d->timer, &QTimer::timeout, this, [=] {
        this->quit();
    });
}

ButtonDiagnostics::~ButtonDiagnostics()
{
    delete ui;
    delete d;
}

void ButtonDiagnostics::quit()
{
    d->gamepad.blockSignals(true);
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit done();
    });
}

void ButtonDiagnostics::on_finishButton_clicked()
{
    this->quit();
}

bool ButtonDiagnostics::event(QEvent*event)
{
    if (event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->gamepad()->deviceId() == d->gamepad.deviceId() && e->isButtonEvent()) {
            if (e->buttonPressed()) {
                d->pressedOrder.enqueue(e->button());
                if (d->pressedOrder.count() > 8) d->pressedOrder.dequeue();
                updateLabel();

                MusicEngine::playSoundEffect(MusicEngine::FocusChanged);
                d->timerButton = e->button();
                d->timer->start();
            } else {
                if (d->timerButton == e->button()) d->timer->stop();
            }
        }
        e->accept();
        return true;
    }
    return false;
}

void ButtonDiagnostics::updateLabel()
{
    QStringList buttons;
    for (QGamepadManager::GamepadButton button : d->pressedOrder) {
        buttons.append(GamepadButtons::stringForButton(button));
    }
    ui->buttonsLabel->setText(buttons.join(" "));
}
