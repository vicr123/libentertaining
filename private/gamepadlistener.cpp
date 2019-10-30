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
#include "gamepadlistener.h"

#include <QGamepadManager>
#include <QApplication>
#include <QWidget>
#include <QKeyEvent>
#include <QScrollArea>
#include <QTimer>
#include <QPointer>
#include "gamepadevent.h"

struct GamepadListenerPrivate {
    QTimer* scrollTimer;
    QPointer<QScrollArea> currentScrollArea;
    double rightAxisX;
    double rightAxisY;
};

GamepadListener::GamepadListener(QObject *parent) : QObject(parent)
{
    d = new GamepadListenerPrivate();

    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this, [=](int deviceId, QGamepadManager::GamepadButton button, double value) {
        GamepadEvent event(deviceId, button, value);
        propagateEvent(&event);
    });
    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this, [=](int deviceId, QGamepadManager::GamepadButton button) {
        GamepadEvent event(deviceId, button, 0);
        propagateEvent(&event);
    });
    connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent, this, [=](int deviceId, QGamepadManager::GamepadAxis axis, double value) {
        GamepadEvent event(deviceId, axis, value);
        propagateEvent(&event);
    });

    d->scrollTimer = new QTimer();
    d->scrollTimer->setInterval(5);
    connect(d->scrollTimer, &QTimer::timeout, this, &GamepadListener::scroll);
}

GamepadListener::~GamepadListener()
{
    delete d;
}

void GamepadListener::scroll()
{
    if (!d->currentScrollArea.isNull()) {
        QPoint gPos = d->currentScrollArea->viewport()->mapToGlobal(QPoint(0, 0));
        QPointF delta(-5 * d->rightAxisX, -5 * d->rightAxisY);

        QWheelEvent event(QPoint(0, 0), gPos, delta.toPoint(), delta.toPoint(), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, true, Qt::MouseEventNotSynthesized);
        QApplication::sendEvent(d->currentScrollArea->viewport(), &event);
    }
}

void GamepadListener::propagateEvent(GamepadEvent*event)
{
    d->currentScrollArea = nullptr;

    QWidget* currentHandling = QApplication::focusWidget();
    if (currentHandling == nullptr) return; //Ignore this event
    while (currentHandling != nullptr) {
        if (!d->currentScrollArea && qobject_cast<QScrollArea*>(currentHandling)) {
            d->currentScrollArea = qobject_cast<QScrollArea*>(currentHandling);
        }

        QApplication::sendEvent(currentHandling, event);
        if (event->isAccepted()) return;
        currentHandling = currentHandling->parentWidget();
    }

    //Synthesise gamepad event
    if (event->buttonPressed()) {
        //Synthesise as a key event
        Qt::Key key;
        if (event->isButtonEvent()) {
            if (QList<QGamepadManager::GamepadButton>({
                QGamepadManager::ButtonUp,
                QGamepadManager::ButtonDown,
                QGamepadManager::ButtonLeft,
                QGamepadManager::ButtonRight,
                QGamepadManager::ButtonA
            }).contains(event->button())) {
                switch (event->button()) {
                    case QGamepadManager::ButtonUp:
                        key = Qt::Key_Up;
                        if (event->axis() == QGamepadManager::AxisRightX) {
                            d->rightAxisX = event->newValue();
                        } else if (event->axis() == QGamepadManager::AxisRightY) {
                            d->rightAxisY = event->newValue();
                        }
                        if (!d->scrollTimer->isActive()) d->scrollTimer->start();
                        break;
                    case QGamepadManager::ButtonDown:
                        key = Qt::Key_Down;
                        break;
                    case QGamepadManager::ButtonLeft:
                        key = Qt::Key_Left;
                        break;
                    case QGamepadManager::ButtonRight:
                        key = Qt::Key_Right;
                        break;
                    case QGamepadManager::ButtonA:
                        key = Qt::Key_Return;
                        break;
                    default:
                        return;
                }
            } else return;
        } else {
            if (event->axis() == QGamepadManager::AxisLeftX) {
                if (event->newValue() < 0) {
                    key = Qt::Key_Left;
                } else {
                    key = Qt::Key_Right;
                }
            } else if (event->axis() == QGamepadManager::AxisLeftY) {
                if (event->newValue() < 0) {
                    key = Qt::Key_Up;
                } else {
                    key = Qt::Key_Down;
                }
            } else return;
        }

        QKeyEvent pressEvent(QKeyEvent::KeyPress, key, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &pressEvent);
        QKeyEvent relEvent(QKeyEvent::KeyRelease, key, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &relEvent);
    }

    if (d->currentScrollArea.isNull()) {
        if (d->scrollTimer->isActive()) d->scrollTimer->stop();
    } else {
        if (event->axis() == QGamepadManager::AxisRightX) {
            d->rightAxisX = event->newValue();
        } else if (event->axis() == QGamepadManager::AxisRightY) {
            d->rightAxisY = event->newValue();
        }

        if (qFuzzyCompare(d->rightAxisX, 0) && qFuzzyCompare(d->rightAxisY, 0)) {
            if (d->scrollTimer->isActive()) d->scrollTimer->stop();
        } else {
            if (!d->scrollTimer->isActive()) d->scrollTimer->start();
        }
    }
}
