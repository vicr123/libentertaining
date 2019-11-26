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
#include "reportwidget.h"
#include "ui_reportwidget.h"

#include <QKeyEvent>
#include "gamepadevent.h"
#include <tvariantanimation.h>
#include "musicengine.h"
#include "online/reportcontroller.h"

struct ReportWidgetPrivate {
    QWidget* parent;
    QGraphicsOpacityEffect* effect;

    QPixmap screenReportPixmap;

    bool isScreenReport;
};

ReportWidget::ReportWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReportWidget)
{
    ui->setupUi(this);
    d = new ReportWidgetPrivate();

    d->effect = new QGraphicsOpacityEffect();
    this->setGraphicsEffect(d->effect);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->mainExplainWidget->setMinimumWidth(SC_DPI(600));
    ui->screenReportContents->setMinimumWidth(SC_DPI(600));

    ui->focusBarrier->setBounceWidget(ui->explainPageContinue);
    ui->focusBarrier_2->setBounceWidget(ui->explainPageContinue);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->backButton->click();
    });

    ui->gamepadHud_2->setButtonText(QGamepadManager::ButtonA, tr("Submit"));
    ui->gamepadHud_2->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud_2->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud_2->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->backButton_2->click();
    });

    ui->mainExplainWidget->setFocusProxy(ui->explainPageContinue);
    ui->screenReportScreen->setFocusProxy(ui->radioButton);
    ui->backButton_2->setFocusProxy(ui->radioButton);
}

ReportWidget::~ReportWidget()
{
    delete ui;
    delete d;
}

void ReportWidget::beginScreenReport(QWidget*widget, QVariantMap details)
{
    d->screenReportPixmap = widget->grab();
    d->isScreenReport = true;

    this->setParent(widget);
    d->parent = widget;
    d->parent->installEventFilter(this);

    ui->screenLabel->setPixmap(d->screenReportPixmap.scaledToWidth(qMin(SC_DPI(600), this->width())));

    this->show();
}

ReportControllerEventFilter::ReportControllerEventFilter() : QObject(nullptr)
{

}

bool ReportControllerEventFilter::eventFilter(QObject*watched, QEvent*event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_F3 && e->modifiers() == Qt::SHIFT) {
            ReportController::beginScreenReport(qobject_cast<QWidget*>(watched));
            e->accept();
            return true;
        }
    } else if (event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->isButtonEvent()) {
            if (e->button() == QGamepadManager::ButtonStart || e->button() == QGamepadManager::ButtonSelect) {

                if (e->buttonPressed()) {
                    if (e->button() == QGamepadManager::ButtonStart) startPressed = true;
                    if (e->button() == QGamepadManager::ButtonSelect) selPressed = true;

                    if (startPressed && selPressed) {
                        ReportController::beginScreenReport(qobject_cast<QWidget*>(watched));
                        startPressed = false;
                        selPressed = false;
                    }
                } else if (e->buttonReleased()) {
                    if (e->button() == QGamepadManager::ButtonStart) startPressed = false;
                    if (e->button() == QGamepadManager::ButtonSelect) selPressed = false;
                }

                e->accept();
                return true;
            }
        }
    }
    return false;
}

void ReportWidget::on_explainPageContinue_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->screenReportScreen);
}

bool ReportWidget::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == d->parent) {
        if (event->type() == QEvent::Resize) {
            this->resize(d->parent->size());

            ui->screenLabel->setPixmap(d->screenReportPixmap.scaledToWidth(qMin(SC_DPI(600), this->width())));
        }
    }
    return false;
}

void ReportWidget::on_backButton_clicked()
{
    this->close();
}

void ReportWidget::on_backButton_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->explainPage);
}

void ReportWidget::show()
{
    this->move(0, SC_DPI(150));
    this->resize(d->parent->size());

    tVariantAnimation* yAnim = new tVariantAnimation();
    yAnim->setStartValue(SC_DPI(150));
    yAnim->setEndValue(0);
    yAnim->setEasingCurve(QEasingCurve::OutCubic);
    yAnim->setDuration(250);
    connect(yAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        this->move(0, value.toInt());
    });
    connect(yAnim, &tVariantAnimation::finished, yAnim, &tVariantAnimation::deleteLater);
    yAnim->start();

    d->effect->setOpacity(0.0);
    tVariantAnimation* oAnim = new tVariantAnimation();
    oAnim->setStartValue(0.0);
    oAnim->setEndValue(1.0);
    oAnim->setEasingCurve(QEasingCurve::OutCubic);
    oAnim->setDuration(250);
    connect(oAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->effect->setOpacity(value.toDouble());
    });
    connect(oAnim, &tVariantAnimation::finished, oAnim, &tVariantAnimation::deleteLater);
    oAnim->start();

    QWidget::show();
    ui->explainPageContinue->setFocus();
}

void ReportWidget::close()
{
    this->setEnabled(false);

    tVariantAnimation* yAnim = new tVariantAnimation();
    yAnim->setStartValue(0);
    yAnim->setEndValue(SC_DPI(150));
    yAnim->setEasingCurve(QEasingCurve::InCubic);
    yAnim->setDuration(250);
    connect(yAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        this->move(0, value.toInt());
    });
    connect(yAnim, &tVariantAnimation::finished, yAnim, &tVariantAnimation::deleteLater);
    yAnim->start();

    tVariantAnimation* oAnim = new tVariantAnimation();
    oAnim->setStartValue(1.0);
    oAnim->setEndValue(0.0);
    oAnim->setEasingCurve(QEasingCurve::InCubic);
    oAnim->setDuration(250);
    connect(oAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->effect->setOpacity(value.toDouble());
    });
    connect(oAnim, &tVariantAnimation::finished, oAnim, &tVariantAnimation::deleteLater);
    connect(oAnim, &tVariantAnimation::finished, this, [=] {
        oAnim->deleteLater();
        emit done();
        QWidget::close();
    });
    oAnim->start();
}
