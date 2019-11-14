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
#include <QStack>
#include <QTimer>
#include <QPointer>
#include <tvariantanimation.h>

struct PauseOverlayPrivate {
    static QMap<QWidget*, PauseOverlay*> overlays;

    QStack<QPointer<QWidget>> overlayWidget;
    QWidget* currentOverlayWidget = nullptr;
    QWidget* parentWidget;

    QWidget* tempFocusWidget;

    QBoxLayout* layout;

    QGraphicsOpacityEffect* opacity;
    QGraphicsOpacityEffect* overlayOpacity;
    QGraphicsBlurEffect* blur;

    bool animatingPop = false;
};

QMap<QWidget*, PauseOverlay*> PauseOverlayPrivate::overlays = QMap<QWidget*, PauseOverlay*>();

PauseOverlay::PauseOverlay(QWidget*blurOver, QWidget *parent) : QWidget(parent)
{
    d = new PauseOverlayPrivate();

    d->tempFocusWidget = new QWidget(this);
    d->tempFocusWidget->setGeometry(-20, -20, 2, 2);

    d->layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    d->layout->setContentsMargins(0, 0, 0, 0);

    d->opacity = new QGraphicsOpacityEffect(this);
    d->opacity->setOpacity(0);
    this->setGraphicsEffect(d->opacity);

    d->blur = new QGraphicsBlurEffect(this);
    d->blur->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    d->blur->setEnabled(false);
    blurOver->setGraphicsEffect(d->blur);

    this->setAttribute(Qt::WA_TranslucentBackground);

    d->parentWidget = parent;
    parent->installEventFilter(this);
    this->setGeometry(0, 0, parent->width(), parent->height());
    this->hide();
}

void PauseOverlay::registerOverlayForWindow(QWidget*window, QWidget*blurOver)
{
    PauseOverlay* overlay = new PauseOverlay(blurOver, window->window());
    PauseOverlayPrivate::overlays.insert(window->window(), overlay);
}

PauseOverlay*PauseOverlay::overlayForWindow(QWidget*window)
{
    return PauseOverlayPrivate::overlays.value(window->window());
}

PauseOverlay::~PauseOverlay()
{
    delete d;
}

void PauseOverlay::showOverlay()
{
    this->show();

    //Hide the focus pointer until we're done
    d->tempFocusWidget->setFocus();

    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->opacity->setOpacity(value.toDouble());
    });
    connect(anim, &tVariantAnimation::finished, this, [=] {
        anim->deleteLater();
        d->opacity->setEnabled(false);
        setNewOverlayWidget(d->overlayWidget.top());
    });
    anim->start();

    d->blur->setEnabled(true);
    tVariantAnimation* blurAnim = new tVariantAnimation(this);
    blurAnim->setStartValue(0.0);
    blurAnim->setEndValue(20.0);
    blurAnim->setDuration(250);
    blurAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(blurAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->blur->setBlurRadius(value.toReal());
    });
    connect(blurAnim, &tVariantAnimation::finished, this, [=] {
        blurAnim->deleteLater();
        d->blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    });
    blurAnim->start();
}

void PauseOverlay::hideOverlay()
{
    d->blur->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->opacity->setOpacity(value.toDouble());
    });
    connect(anim, &tVariantAnimation::finished, this, [=] {
        anim->deleteLater();
        this->hide();

//        tVariantAnimation* blurAnim = new tVariantAnimation(this);
//        blurAnim->setStartValue(20.0);
//        blurAnim->setEndValue(0.0);
//        blurAnim->setDuration(250);
//        blurAnim->setEasingCurve(QEasingCurve::OutCubic);
//        connect(blurAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
//            d->blur->setBlurRadius(value.toReal());
//        });
//        connect(blurAnim, &tVariantAnimation::finished, this, [=] {
//            d->blur->setEnabled(false);
//            blurAnim->deleteLater();
//        });
//        blurAnim->start();
        d->blur->setEnabled(false);

    });
    anim->start();
}

void PauseOverlay::setOverlayWidget(QWidget*overlayWidget)
{
    d->overlayWidget.clear();
    pushOverlayWidget(overlayWidget);
}

void PauseOverlay::pushOverlayWidget(QWidget*overlayWidget)
{
    d->overlayWidget.push(overlayWidget);

    //Don't do anything if we're currently animating a pop
    //We'll automatically show this widget when the pop completes
    if (!d->animatingPop) {
        if (this->isVisible()) {
            animateCurrentOut([=] {
                setNewOverlayWidget(overlayWidget);
            });
        } else {
            this->showOverlay();
        }
    }
}

void PauseOverlay::popOverlayWidget(std::function<void ()> after)
{
    //If we're already popping we want to go back more steps
    if (d->animatingPop) {
        QWidget* topWidget = d->overlayWidget.pop();
        topWidget->setGraphicsEffect(nullptr);

        QMetaObject::Connection* c = new QMetaObject::Connection();
        *c = connect(this, &PauseOverlay::widgetPopped, this, [=] {
            disconnect(*c);
            delete c;

            after();
        });

        return;
    }

    d->animatingPop = true;
    QWidget* topWidget = d->overlayWidget.pop();

    animateCurrentOut([=] {
        topWidget->setGraphicsEffect(nullptr);
        after();
        emit widgetPopped();

        QTimer::singleShot(0, this, [=] {
            d->animatingPop = false;

            if (d->overlayWidget.count() > 0) {
                setNewOverlayWidget(d->overlayWidget.top());
            } else {
                this->hideOverlay();
            }
        });
    });
}

bool PauseOverlay::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == d->parentWidget && event->type() == QEvent::Resize) {
        this->resize(d->parentWidget->width(), d->parentWidget->height());
    }
    return false;
}

void PauseOverlay::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 200));
    painter.setPen(Qt::transparent);
    painter.drawRect(0, 0, this->width(), this->height());
}

void PauseOverlay::animateCurrentOut(std::function<void ()> after)
{
    //Hide the focus pointer until we're done
    d->tempFocusWidget->setFocus();
    d->overlayOpacity->setEnabled(true);

    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->overlayOpacity->setOpacity(value.toDouble());
    });
    connect(anim, &tVariantAnimation::finished, this, [=] {
        anim->deleteLater();
        d->layout->removeWidget(d->currentOverlayWidget);
        d->currentOverlayWidget->setVisible(false);
        after();
    });
    anim->start();
}

void PauseOverlay::setNewOverlayWidget(QWidget*widget, std::function<void()> after)
{
    d->currentOverlayWidget = widget;
    d->layout->addWidget(widget);

    d->overlayOpacity = new QGraphicsOpacityEffect(this);
    d->overlayOpacity->setOpacity(0);

    widget->setGraphicsEffect(d->overlayOpacity);
    widget->setVisible(true);

    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->overlayOpacity->setOpacity(value.toDouble());
        this->update();
    });
    connect(anim, &tVariantAnimation::finished, this, [=] {
        anim->deleteLater();
        this->update();
        d->overlayOpacity->setEnabled(false);
        widget->setFocus();
        after();
    });
    anim->start();
}
