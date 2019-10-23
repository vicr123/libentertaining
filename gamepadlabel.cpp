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
#include "gamepadlabel.h"

#include <QPainter>
#include "gamepadbuttons.h"

GamepadLabel::GamepadLabel(QWidget *parent) : QLabel(parent)
{

}

void GamepadLabel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setFont(this->font());
    painter.setPen(this->palette().color(QPalette::WindowText));

    QRect boundingRect(0, 0, this->width(), this->height());
    boundingRect.adjust(this->margin(), this->margin(), -this->margin(), -this->margin());
    GamepadButtons::drawGamepadString(&painter, this->text(), boundingRect);
}

QSize GamepadLabel::sizeHint() const
{
    QSize size;
    size.setHeight(this->fontMetrics().height());
    size.setWidth(GamepadButtons::measureGamepadString(this->fontMetrics(), this->text()));
    size.rheight() += this->margin() * 2;
    size.rwidth() += this->margin() * 2;
    return size;
}

QSize GamepadLabel::minimumSizeHint() const
{
    return sizeHint();
}
