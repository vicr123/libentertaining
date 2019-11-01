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
#ifndef GAMEPADEVENT_H
#define GAMEPADEVENT_H

#include <QEvent>
#include <QGamepadManager>
#include "libentertaining_global.h"

struct GamepadEventPrivate;
class LIBENTERTAINING_EXPORT GamepadEvent : public QEvent
{
    public:
        explicit GamepadEvent(int deviceId, QGamepadManager::GamepadButton button, double value);
        explicit GamepadEvent(int deviceId, QGamepadManager::GamepadAxis axis, double value);
        ~GamepadEvent();

        static QEvent::Type type();

        QGamepad* gamepad();
        double newValue();
        QPointF newAxisLocation();

        bool isButtonEvent();

        QGamepadManager::GamepadButton button();
        bool buttonPressed();
        bool buttonReleased();

        QGamepadManager::GamepadAxis axis();

    private:
        GamepadEventPrivate* dd;
};

#endif // GAMEPADEVENT_H
