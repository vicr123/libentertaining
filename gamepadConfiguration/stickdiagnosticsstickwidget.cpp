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
#include "stickdiagnosticsstickwidget.h"

#include <QPainter>
#include <the-libs_global.h>

struct StickDiagnosticsStickWidgetPrivate {
    QString side;
    double x = 0, y = 0;
};

StickDiagnosticsStickWidget::StickDiagnosticsStickWidget(QWidget *parent) : QWidget(parent)
{
    d = new StickDiagnosticsStickWidgetPrivate();
}

StickDiagnosticsStickWidget::~StickDiagnosticsStickWidget()
{
    delete d;
}

QSize StickDiagnosticsStickWidget::sizeHint() const
{
    return SC_DPI_T(QSize(128, 128), QSize);
}

void StickDiagnosticsStickWidget::setSide(QString side)
{
    d->side = side;
}

void StickDiagnosticsStickWidget::setXAxis(double x)
{
    d->x = x;
    this->update();
}

void StickDiagnosticsStickWidget::setYAxis(double y)
{
    d->y = y;
    this->update();
}

void StickDiagnosticsStickWidget::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.setBrush(Qt::transparent);

    //Draw the side
    painter.setOpacity(0.3);

    QFont fnt = this->font();
    fnt.setPixelSize(static_cast<int>(this->height() * 0.6));

    painter.setFont(fnt);
    painter.drawText(0, 0, this->width(), this->height(), Qt::AlignCenter, d->side);
    painter.setOpacity(1);

    //Draw the outside circle
    QSize oneSize(this->width(), this->height());
    oneSize *= 0.9;

    QRect circleArea;
    circleArea.setSize(oneSize);
    circleArea.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    painter.drawEllipse(circleArea);

    //Draw the inner lines
    painter.drawLine(this->width() / 2, 0, this->width() / 2, this->height());
    painter.drawLine(0, this->height() / 2, this->width(), this->height() / 2);


    painter.setBrush(this->palette().color(QPalette::WindowText));

    QPoint stickCenter;
    stickCenter.setX(static_cast<int>(this->width() / 2 + (d->x * (this->width() / 2 * 0.9))));
    stickCenter.setY(static_cast<int>(this->height() / 2 + (d->y * (this->height() / 2 * 0.9))));

    QRect stickRect;
    stickRect.setSize(SC_DPI_T(QSize(5, 5), QSize));
    stickRect.moveCenter(stickCenter);
    painter.drawEllipse(stickRect);
}
