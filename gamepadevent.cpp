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
#include "gamepadevent.h"

#include <QGamepad>

struct GamepadEventPrivate {
    static QEvent::Type eventType;

    QGamepad* gamepad;
    double value;

    QGamepadManager::GamepadButton button = QGamepadManager::ButtonInvalid;
    QGamepadManager::GamepadAxis axis = QGamepadManager::AxisInvalid;

    static QMap<QGamepadManager::GamepadButton, double> buttonValues;
    static QMap<QGamepadManager::GamepadAxis, double> axisValues;
    bool buttonPressed = false;
    bool buttonReleased = false;
};

QEvent::Type GamepadEventPrivate::eventType = QEvent::None;
QMap<QGamepadManager::GamepadButton, double> GamepadEventPrivate::buttonValues = QMap<QGamepadManager::GamepadButton, double>();
QMap<QGamepadManager::GamepadAxis, double> GamepadEventPrivate::axisValues = QMap<QGamepadManager::GamepadAxis, double>();

GamepadEvent::GamepadEvent(int deviceId, QGamepadManager::GamepadButton button, double value) : QEvent(GamepadEvent::type())
{
    this->setAccepted(false);

    dd = new GamepadEventPrivate();

    dd->gamepad = new QGamepad(deviceId);
    dd->value = value;
    dd->button = button;

    double oldValue = dd->buttonValues.value(button, 0);
    if (oldValue < 0.8 && value >= 0.8) {
        dd->buttonPressed = true;
    } else if (oldValue >= 0.8 && value < 0.8) {
        dd->buttonReleased = true;
    }
    dd->buttonValues.insert(button, value);
}

GamepadEvent::GamepadEvent(int deviceId, QGamepadManager::GamepadAxis axis, double value) : QEvent(GamepadEvent::type())
{
    this->setAccepted(false);

    dd = new GamepadEventPrivate();

    dd->gamepad = new QGamepad(deviceId);
    dd->value = value;
    dd->axis = axis;

    double oldValue = dd->axisValues.value(axis, 0);
    if (abs(oldValue) < 0.8 && abs(value) >= 0.8) {
        dd->buttonPressed = true;
    } else if (abs(oldValue) >= 0.8 && abs(value) < 0.8) {
        dd->buttonReleased = true;
    }
    dd->axisValues.insert(axis, value);
}

GamepadEvent::~GamepadEvent()
{
    dd->gamepad->deleteLater();
    delete dd;
}

QEvent::Type GamepadEvent::type()
{
    if (GamepadEventPrivate::eventType == QEvent::None) {
        GamepadEventPrivate::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
    }
    return GamepadEventPrivate::eventType;
}

QGamepad* GamepadEvent::gamepad()
{
    return dd->gamepad;
}

double GamepadEvent::newValue()
{
    return dd->value;
}

bool GamepadEvent::isButtonEvent()
{
    return dd->button != QGamepadManager::ButtonInvalid;
}

QGamepadManager::GamepadButton GamepadEvent::button()
{
    return dd->button;
}

bool GamepadEvent::buttonPressed()
{
    return dd->buttonPressed;
}

bool GamepadEvent::buttonReleased()
{
    return dd->buttonReleased;
}

QGamepadManager::GamepadAxis GamepadEvent::axis()
{
    return dd->axis;
}
