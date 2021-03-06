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
#ifndef KEYBOARDLAYOUTSDATABASE_H
#define KEYBOARDLAYOUTSDATABASE_H

#include <QCoreApplication>
#include <QMap>
#include "keyboard.h"

class KeyboardLayoutsModel;
class KeyboardLayoutsDatabase
{
        Q_DECLARE_TR_FUNCTIONS(KeyboardLayoutsDatabase)

    public:
        static KeyboardLayout layoutForName(QString name);

    protected:
        friend KeyboardLayoutsModel;
        static QMap<QString, KeyboardLayout> displayLayouts();

    private:
        KeyboardLayoutsDatabase();
        static QMap<QString, KeyboardLayout> layouts;
        static QMap<QString, KeyboardLayout> setupLayouts();
};

#endif // KEYBOARDLAYOUTSDATABASE_H
