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
#include "applicationeventfilter.h"

#include <QApplication>
#include "gamepadevent.h"

ApplicationEventFilter::ApplicationEventFilter(QObject *parent) : QObject(parent)
{

}

bool ApplicationEventFilter::eventFilter(QObject*watched, QEvent*event)
{
    if (event->type() == QEvent::MouseMove && isCursorHidden) {
        QApplication::restoreOverrideCursor();
        isCursorHidden = false;
    } else if ((event->type() == QEvent::KeyPress || event->type() == GamepadEvent::type()) && !isCursorHidden) {
        QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
        isCursorHidden = true;
    }
    return false;
}
