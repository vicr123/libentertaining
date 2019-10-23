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
#include "pauseoverlay.h"

#include <QPainter>
#include <QBoxLayout>
#include <QEvent>
#include <QGraphicsBlurEffect>

struct PauseOverlayPrivate {
    QWidget* overlayWidget;
    QWidget* overlayOver;

    QGraphicsBlurEffect* effect;
};

PauseOverlay::PauseOverlay(QWidget*overlayWidget, QWidget *parent) : QWidget(parent)
{
    d = new PauseOverlayPrivate();
    d->overlayWidget = overlayWidget;

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(overlayWidget);

    d->effect = new QGraphicsBlurEffect();
    d->effect->setBlurRadius(20);

    this->setAttribute(Qt::WA_TranslucentBackground);
}

PauseOverlay::~PauseOverlay()
{
    delete d;
}

void PauseOverlay::showOverlay(QWidget*overlayOver)
{
    d->overlayOver = overlayOver;
    overlayOver->installEventFilter(this);

    if (d->overlayOver->parentWidget() != nullptr) {
        this->setGeometry(overlayOver->geometry());
        this->setParent(overlayOver->parentWidget());
        overlayOver->setGraphicsEffect(d->effect);
    } else {
        this->setGeometry(QRect(QPoint(0, 0), overlayOver->size()));
        this->setParent(overlayOver);
    }
    this->show();
}

void PauseOverlay::hideOverlay()
{
    this->hide();
    this->setParent(nullptr);

    if (d->overlayOver->parentWidget() != nullptr) {
        d->overlayOver->setGraphicsEffect(nullptr);
    }
}

bool PauseOverlay::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == d->overlayOver && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        if (d->overlayOver->parentWidget() != nullptr) {
            this->setGeometry(d->overlayOver->geometry());
        } else {
            this->resize(d->overlayOver->width(), d->overlayOver->height());
        }
    }
    return false;
}

void PauseOverlay::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 127));
    painter.setPen(Qt::transparent);
    painter.drawRect(0, 0, this->width(), this->height());
}
