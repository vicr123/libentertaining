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
#include "keyboardlayoutsdatabase.h"

QMap<QString, KeyboardLayout> KeyboardLayoutsDatabase::layouts = KeyboardLayoutsDatabase::setupLayouts();

KeyboardLayoutsDatabase::KeyboardLayoutsDatabase()
{
}

QMap<QString, KeyboardLayout> KeyboardLayoutsDatabase::setupLayouts()
{
    //Set up keyboard layouts
    QList<KeyboardLayout> layouts;

    {
        KeyboardLayout usLayout;
        usLayout.name = "en-US";
        usLayout.keys = {
            {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', KeyboardKey::Backspace},
            {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\''},
            {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '/', KeyboardKey::Ok},
            {KeyboardKey::Shift, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', KeyboardKey::Shift},
            {KeyboardKey::SetNumeric, KeyboardKey::Space, KeyboardKey::SetLayout}
        };
        layouts.append(usLayout);
    }

    {
        KeyboardLayout numLayout;
        numLayout.name = "numOnly";
        numLayout.honourShift = false;
        numLayout.keys = {
            {'1', '2', '3', KeyboardKey::Backspace},
            {'4', '5', '6', KeyboardKey::Ok},
            {'7', '8', '9', KeyboardKey::Dummy},
            {KeyboardKey::Dummy, '0', KeyboardKey::Dummy, KeyboardKey::Dummy}
        };
        layouts.append(numLayout);
    }

    QMap<QString, KeyboardLayout> layoutsMap;
    for (KeyboardLayout layout : layouts) {
        layoutsMap.insert(layout.name, layout);
    }
    return layoutsMap;
}

KeyboardLayout KeyboardLayoutsDatabase::layoutForName(QString name)
{
    return layouts.value(name);
}
