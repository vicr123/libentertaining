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
#ifndef GAMEPADBUTTONS_H
#define GAMEPADBUTTONS_H

#include "libentertaining_global.h"
#include <QObject>
#include <QGamepadManager>

class QPainter;
class QFontMetrics;

class LIBENTERTAINING_EXPORT GamepadButtons : public QObject
{
        Q_OBJECT
    public:
        static QIcon iconForButton(QGamepadManager::GamepadButton button, QColor tint);
        static QString stringForButton(QGamepadManager::GamepadButton button);

        static QPixmap iconForKey(QKeySequence key, QFont font, QPalette pal);
        static QString stringForKey(QKeySequence key);

        static int measureGamepadString(QFont font, QString string);
        static void drawGamepadString(QPainter* painter, QString string, QRect boundingRect, QPalette pal);

    signals:

    public slots:

    private:
        explicit GamepadButtons(QObject *parent = nullptr);

        static QPixmap getKeyIcon(QString key, QFont font, QPalette pal);
};


#endif // GAMEPADBUTTONS_H
