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
#include "focusbarrier.h"

#include "musicengine.h"

struct FocusBarrierPrivate {
    QWidget* bounceWidget = nullptr;
};

FocusBarrier::FocusBarrier(QWidget *parent) : QPushButton(parent)
{
    d = new FocusBarrierPrivate();
    this->setFixedHeight(0);
    this->setFixedWidth(0);
}

FocusBarrier::~FocusBarrier()
{
    delete d;
}

void FocusBarrier::setBounceWidget(QWidget*widget)
{
    d->bounceWidget = widget;
}

void FocusBarrier::focusInEvent(QFocusEvent*event)
{
    MusicEngine::playSoundEffect(MusicEngine::FocusChangedFailed);
    if (d->bounceWidget) d->bounceWidget->setFocus();
}
