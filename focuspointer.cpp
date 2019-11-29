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
#include <QTimer>
#include <QAbstractItemView>
#include <the-libs_global.h>
#include <tvariantanimation.h>

#include "focusbarrier.h"
#include "musicengine.h"

struct FocusPointerPrivate {
    FocusPointer* instance = nullptr;
    QPointer<QWidget> activeWidget;
    QList<QPointer<QWidget>> filteredWidgets;

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
            disconnect(d->activeWidget, nullptr, this, nullptr);
            d->activeWidget->removeEventFilter(this);

            for (QPointer<QWidget> w : d->filteredWidgets) {
                if (!w.isNull()) {
                    disconnect(w, nullptr, this, nullptr);
                    w->removeEventFilter(this);
                }
            }
        }

        d->activeWidget = now;

        if (d->activeWidget.isNull()) {
            d->instance->hide();
        } else {
            this->setFocusProxy(d->activeWidget);
            d->activeWidget->installEventFilter(this);
            QWidget* parent = d->activeWidget->parentWidget();
            while (parent != nullptr) {
                parent->installEventFilter(this);
                d->filteredWidgets.append(QPointer<QWidget>(parent));
                parent = parent->parentWidget();
            }

            QAbstractItemView* listView = qobject_cast<QAbstractItemView*>(d->activeWidget);
            if (listView) {
                connect(listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=] {
                    MusicEngine::playSoundEffect(MusicEngine::FocusChanged);
                    QTimer::singleShot(0, this, &FocusPointer::updateFocusedWidget);
                });
            }

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

//    QColor mainCol = QApplication::palette().color(QPalette::Highlight);

    d->colourPulse = new tVariantAnimation();
    d->colourPulse->setForceAnimation(true);
    d->colourPulse->setStartValue(QColor(0, 100, 255));
    d->colourPulse->setEndValue(QColor(0, 200, 255));
//    d->colourPulse->setStartValue(mainCol.darker());
//    d->colourPulse->setEndValue(mainCol.lighter());
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

    if (d->activeWidget.isNull() || d->activeWidget->window() == nullptr) {
        this->setParent(nullptr);
        d->instance->hide();
    } else {
        QWidget* parentWindow = d->activeWidget->window();
        if (d->enabled) d->instance->show();

        this->setParent(parentWindow);

        QRect geometry;

        QAbstractItemView* listView = qobject_cast<QAbstractItemView*>(d->activeWidget);
        if (listView) {
            QModelIndex index = listView->currentIndex();
            if (index.isValid()) {
                geometry = listView->visualRect(index);
                geometry.moveTopLeft(listView->viewport()->mapTo(parentWindow, geometry.topLeft()));
                geometry.adjust(SC_DPI(-5), SC_DPI(-5), SC_DPI(5), SC_DPI(5));
            }
        }

        if (geometry.isNull()) {
            geometry.setSize(d->activeWidget->size() + SC_DPI_T(QSize(10, 10), QSize));
            geometry.moveTopLeft(d->activeWidget->mapTo(parentWindow, SC_DPI_T(QPoint(-5, -5), QPoint)));
        }

        this->setGeometry(geometry);
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

    if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
        if (watched == d->activeWidget) {
            updateFocusedWidget();
        }

        if (event->type() == QEvent::Move && d->filteredWidgets.contains(QPointer<QWidget>(qobject_cast<QWidget*>(watched)))) {
            updateFocusedWidget();
        }
    }
    return false;
}
