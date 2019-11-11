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

#include <qmath.h>
#include <QGamepad>
#include <QPointF>

struct GamepadEventPrivate {
    static QEvent::Type eventType;

    QGamepad* gamepad;
    double value;

    QGamepadManager::GamepadButton button = QGamepadManager::ButtonInvalid;
    QGamepadManager::GamepadAxis axis = QGamepadManager::AxisInvalid;

    static QMap<QPair<int, QGamepadManager::GamepadButton>, double> buttonValues;
    static QMap<QPair<int, QGamepadManager::GamepadAxis>, double> axisValues;
    static QMap<QPair<int, QGamepadManager::GamepadAxis>, double> realAxisValues;
    bool buttonPressed = false;
    bool buttonReleased = false;

    QPointF axisLocation;
};

QEvent::Type GamepadEventPrivate::eventType = QEvent::None;
QMap<QPair<int, QGamepadManager::GamepadButton>, double> GamepadEventPrivate::buttonValues = QMap<QPair<int, QGamepadManager::GamepadButton>, double>();
QMap<QPair<int, QGamepadManager::GamepadAxis>, double> GamepadEventPrivate::axisValues = QMap<QPair<int, QGamepadManager::GamepadAxis>, double>();
QMap<QPair<int, QGamepadManager::GamepadAxis>, double> GamepadEventPrivate::realAxisValues = QMap<QPair<int, QGamepadManager::GamepadAxis>, double>();

GamepadEvent::GamepadEvent(int deviceId, QGamepadManager::GamepadButton button, double value) : QEvent(GamepadEvent::type())
{
    this->setAccepted(false);

    dd = new GamepadEventPrivate();

    dd->gamepad = new QGamepad(deviceId);
    dd->value = value;
    dd->button = button;

    double oldValue = dd->buttonValues.value({deviceId, button}, 0);
    if (oldValue < 0.8 && value >= 0.8) {
        dd->buttonPressed = true;
    } else if (oldValue >= 0.8 && value < 0.8) {
        dd->buttonReleased = true;
    }
    dd->buttonValues.insert({deviceId, button}, value);
}

GamepadEvent::GamepadEvent(int deviceId, QGamepadManager::GamepadAxis axis, double value) : QEvent(GamepadEvent::type())
{
    this->setAccepted(false);

    dd = new GamepadEventPrivate();

    dd->gamepad = new QGamepad(deviceId);
    dd->axis = axis;
    dd->realAxisValues.insert({deviceId, axis}, value);

    //Normalise the values of the gamepad
    qreal x, y;


    switch (axis) {
        case QGamepadManager::AxisLeftX:
            x = value;
            y = dd->realAxisValues.value({deviceId, QGamepadManager::AxisLeftY}, 0);
            break;
        case QGamepadManager::AxisLeftY:
            x = dd->realAxisValues.value({deviceId, QGamepadManager::AxisLeftX}, 0);
            y = value;
            break;
        case QGamepadManager::AxisRightX:
            x = value;
            y = dd->realAxisValues.value({deviceId, QGamepadManager::AxisRightY}, 0);
            break;
        case QGamepadManager::AxisRightY:
            x = dd->realAxisValues.value({deviceId, QGamepadManager::AxisRightX}, 0);
            y = value;
            break;
        case QGamepadManager::AxisInvalid:
            x = 0;
            y = 0;
            break;
    }

    if (qFuzzyIsNull(x) && qFuzzyIsNull(y)) {
        dd->value = 0;
    } else {
        bool shouldFlipX = false, shouldFlipY = false;
        if (x < 0) {
            shouldFlipX = true;
            x *= -1;
        }
        if (y < 0) {
            shouldFlipY = true;
            y *= -1;
        }

        qreal angle = qAtan(y / x);
        qreal projected = qMin(1 / qCos(angle), 1 / qSin(angle));
        qreal r = qSqrt(y * y + x * x);
        qreal normR = r / projected * 10.0 / 9.0; //Overscan the radius by 10/9

        qreal normX = normR * qCos(angle);
        qreal normY = normR * qSin(angle);

//        qreal normX = x;
//        qreal normY = y;

        if (shouldFlipX) normX *= -1;
        if (shouldFlipY) normY *= -1;


        dd->axisLocation = QPointF(normX, normY);

        switch (axis) {
            case QGamepadManager::AxisLeftX:
            case QGamepadManager::AxisRightX:
                dd->value = normX;
                break;
            case QGamepadManager::AxisLeftY:
            case QGamepadManager::AxisRightY:
                dd->value = normY;
                break;
            case QGamepadManager::AxisInvalid:
                break;
        }
    }

    double oldValue = dd->axisValues.value({deviceId, axis}, 0);
    if (qAbs(oldValue) < 1 && qAbs(dd->value) >= 1) {
        dd->buttonPressed = true;
    } else if (qAbs(oldValue) >= 1 && qAbs(dd->value) < 1) {
        dd->buttonReleased = true;
    }
    dd->axisValues.insert({deviceId, axis}, dd->value);
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

QPointF GamepadEvent::newAxisLocation()
{
    return dd->axisLocation;
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
