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
#include "focuspointer.h"

#include <QApplication>
#include <QPointer>
#include <QPainter>
#include <the-libs_global.h>
#include <tvariantanimation.h>

#include "focusbarrier.h"
#include "musicengine.h"

struct FocusPointerPrivate {
    FocusPointer* instance = nullptr;
    QPointer<QWidget> activeWidget;

    bool automatic = false;
    bool enabled = false;

    tVariantAnimation* colourPulse;

    void ensureInstance() {
        if (instance == nullptr) instance = new FocusPointer();
    }
};

FocusPointerPrivate* FocusPointer::d = new FocusPointerPrivate();

void FocusPointer::enableFocusPointer()
{
    d->ensureInstance();
    d->enabled = true;
}

void FocusPointer::disableFocusPointer()
{
    d->ensureInstance();
    d->instance->hide();
    d->enabled = false;
}

void FocusPointer::enableAutomaticFocusPointer()
{
    d->ensureInstance();
    d->automatic = true;
}

void FocusPointer::disableAutomaticFocusPointer()
{
    d->ensureInstance();
    d->automatic = false;
}

bool FocusPointer::isEnabled()
{
    return d->enabled;
}

FocusPointer::FocusPointer() : QWidget(nullptr)
{
    this->setAttribute(Qt::WA_TransparentForMouseEvents);

    connect(static_cast<QApplication*>(QApplication::instance()), &QApplication::focusChanged, this, [=](QWidget* old, QWidget* now) {
        if (!d->activeWidget.isNull()) {
            d->activeWidget->removeEventFilter(this);
        }

        d->activeWidget = now;

        if (d->activeWidget.isNull()) {
            d->instance->hide();
        } else {
            this->setFocusProxy(d->activeWidget);
            d->activeWidget->installEventFilter(this);


            if (d->enabled) {
                if (qobject_cast<FocusBarrier*>(d->activeWidget) == nullptr && qobject_cast<FocusBarrier*>(old) == nullptr) {
                    MusicEngine::playSoundEffect(MusicEngine::FocusChanged);
                }
            }

            this->updateFocusedWidget();
        }
    });
    QApplication::instance()->installEventFilter(this);

    this->resize(SC_DPI_T(QSize(32, 32), QSize));

    d->colourPulse = new tVariantAnimation();
    d->colourPulse->setForceAnimation(true);
    d->colourPulse->setStartValue(QColor(0, 100, 255));
    d->colourPulse->setEndValue(QColor(0, 200, 255));
    d->colourPulse->setDuration(500);
    connect(d->colourPulse, &tVariantAnimation::valueChanged, this, [=] {
        this->update();
    });
    connect(d->colourPulse, &tVariantAnimation::finished, this, [=] {
        if (d->colourPulse->direction() == tVariantAnimation::Forward) {
            d->colourPulse->setDirection(tVariantAnimation::Backward);
        } else {
            d->colourPulse->setDirection(tVariantAnimation::Forward);
        }
        d->colourPulse->start();
    });
    d->colourPulse->start();

    this->hide();
}

void FocusPointer::updateFocusedWidget()
{
    QWidget* parentWindow = d->activeWidget->window();

    if (d->activeWidget.isNull() || parentWindow == nullptr) {
        this->setParent(nullptr);
        d->instance->hide();
    } else {
        if (d->enabled) d->instance->show();

        this->setParent(parentWindow);
        this->resize(d->activeWidget->size() + SC_DPI_T(QSize(10, 10), QSize));
        this->move(d->activeWidget->mapTo(parentWindow, SC_DPI_T(QPoint(-5, -5), QPoint)));
//        this->move(d->activeWidget->mapTo(parentWindow, QPoint(d->activeWidget->width() - SC_DPI(16), d->activeWidget->height() - SC_DPI(16))));

        this->raise();
    }
}

void FocusPointer::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);
//    painter.setBrush(QColor(0, 0, 0, 127));
//    painter.setPen(Qt::transparent);
//    painter.setBrush(Qt::transparent);
//    painter.setPen(QPen(QColor(255, 255, 255), SC_DPI(5), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    painter.setPen(QPen(QColor(0, 150, 255), SC_DPI(10), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    painter.drawRect(5, 5, this->width() + 10, this->height() + 10);

    painter.setBrush(d->colourPulse->currentValue().value<QColor>());
    painter.setPen(Qt::transparent);
    painter.drawRect(0, 0, this->width(), SC_DPI(5));
    painter.drawRect(0, 0, SC_DPI(5), this->height());
    painter.drawRect(0, this->height() - SC_DPI(5), this->width(), SC_DPI(5));
    painter.drawRect(this->width() - SC_DPI(5), 0, SC_DPI(5), this->height());

//    QPolygon pol;
//    pol.append(QPoint(0, 0));
//    pol.append(QPoint(this->width(), this->height() / 2));
//    pol.append(QPoint(this->width() / 2, this->height() / 2));
//    pol.append(QPoint(this->width() / 2, this->height()));
//    pol.append(QPoint(0, 0));


//    painter.setPen(Qt::black);
//    painter.setBrush(Qt::white);
//    painter.drawPolygon(pol);
}

bool FocusPointer::eventFilter(QObject*watched, QEvent*event)
{
    if (d->automatic) {
        if (event->type() == QEvent::MouseMove) {
            this->disableFocusPointer();
        } else if (event->type() == QEvent::KeyPress) {
            this->enableFocusPointer();
        }
    }

    if (watched == d->activeWidget) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            updateFocusedWidget();
        }
    }
    return false;
}
