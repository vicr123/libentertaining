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
#ifndef GAMEPADHUD_H
#define GAMEPADHUD_H

#include <QWidget>
#include <QGamepad>
#include "libentertaining_global.h"

namespace Ui {
    class GamepadHud;
}

struct GamepadHudPrivate;
class LIBENTERTAINING_EXPORT GamepadHud : public QWidget
{
        Q_OBJECT

    public:
        explicit GamepadHud(QWidget *parent = nullptr);
        ~GamepadHud();

        enum StandardAction {
            SelectAction
        };

        void setParent(QWidget* parent);
        void addListener(QWidget* listenTo);
        void removeListener(QWidget* listenTo);

        void setButtonText(QGamepadManager::GamepadButton button, QString text);
        void setButtonAction(QGamepadManager::GamepadButton button, std::function<void()> action);
        void removeText(QGamepadManager::GamepadButton button);
        void removeButtonAction(QGamepadManager::GamepadButton button);
        void bindKey(QKeySequence key, QGamepadManager::GamepadButton button);
        void unbindKey(QKeySequence key);

        static std::function<void()> standardAction(StandardAction action);

    private:
        Ui::GamepadHud *ui;
        GamepadHudPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);
        void setShowGamepadButtons(bool showGamepadButtons);
};

#endif // GAMEPADHUD_H
