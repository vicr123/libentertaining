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
#include "textinputlineedithandler.h"

#include <QApplication>
#include <QKeyEvent>
#include "gamepadevent.h"

TextInputLineEditHandler::TextInputLineEditHandler(QLineEdit *parent) : QObject(parent)
{
    parent->installEventFilter(this);
}

TextInputLineEditHandler::~TextInputLineEditHandler()
{

}

bool TextInputLineEditHandler::eventFilter(QObject*watched, QEvent*event)
{
    if (event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->isButtonEvent() && e->buttonPressed() && e->button() == QGamepadManager::ButtonA) {
            //Open the keyboard
            emit openKeyboard();

            //Prevent propagation
            e->accept();
            return true;
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
            //Move the focus
            Qt::Key key = e->key() == Qt::Key_Up ? Qt::Key_Backtab : Qt::Key_Tab;

            QKeyEvent pressEvent(QKeyEvent::KeyPress, key, Qt::NoModifier);
            QApplication::sendEvent(QApplication::focusWidget(), &pressEvent);
            QKeyEvent relEvent(QKeyEvent::KeyRelease, key, Qt::NoModifier);
            QApplication::sendEvent(QApplication::focusWidget(), &relEvent);
        }

    }
    return false;
}
