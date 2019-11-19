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
#include "horizontalspinbox.h"
#include "ui_horizontalspinbox.h"

#include <QIntValidator>
#include <QKeyEvent>
#include "musicengine.h"

struct HorizontalSpinBoxPrivate {
    int value = 0;
    int max = 99;
    int min = 0;
    QIntValidator validator;

    bool ignoreEvents = false;
};

HorizontalSpinBox::HorizontalSpinBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HorizontalSpinBox)
{
    ui->setupUi(this);
    d = new HorizontalSpinBoxPrivate();

    this->setFocusPolicy(Qt::StrongFocus);
    ui->lineEdit->setText(QString::number(d->value));
    ui->lineEdit->setValidator(&d->validator);

    d->validator.setRange(d->min, d->max);
}

HorizontalSpinBox::~HorizontalSpinBox()
{
    delete d;
    delete ui;
}

void HorizontalSpinBox::setValue(int value)
{
    d->value = value;
    ui->lineEdit->setText(QString::number(d->value));
    emit valueChanged(value);
}

int HorizontalSpinBox::value()
{
    return d->value;
}

int HorizontalSpinBox::max()
{
    return d->max;
}

int HorizontalSpinBox::min()
{
    return d->min;
}

void HorizontalSpinBox::setRange(int min, int max)
{
    d->min = min;
    d->max = max;
    d->validator.setRange(min, max);
}

void HorizontalSpinBox::on_upButton_clicked()
{
    if (!this->isEnabled()) return;
    int n = d->value + 1;
    if (n > d->max) n = d->max;
    this->setValue(n);
}

void HorizontalSpinBox::on_downButton_clicked()
{
    if (!this->isEnabled()) return;
    int n = d->value - 1;
    if (n < d->min) n = d->min;
    this->setValue(n);
}

void HorizontalSpinBox::keyPressEvent(QKeyEvent*event)
{
    if (d->ignoreEvents) return;

    d->ignoreEvents = true;
    if (event->key() == Qt::Key_Left) {
        MusicEngine::playSoundEffect(MusicEngine::FocusChanged);
        ui->downButton->click();
    } else if (event->key() == Qt::Key_Right) {
        MusicEngine::playSoundEffect(MusicEngine::FocusChanged);
        ui->upButton->click();
    } else if (event->key() == Qt::Key_Up) {
        QKeyEvent pressEvent(QKeyEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
        QApplication::sendEvent(this, &pressEvent);
        QKeyEvent relEvent(QKeyEvent::KeyRelease, Qt::Key_Backtab, Qt::NoModifier);
        QApplication::sendEvent(this, &relEvent);
    } else if (event->key() == Qt::Key_Down) {
        QKeyEvent pressEvent(QKeyEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        QApplication::sendEvent(this, &pressEvent);
        QKeyEvent relEvent(QKeyEvent::KeyRelease, Qt::Key_Tab, Qt::NoModifier);
        QApplication::sendEvent(this, &relEvent);
    } else if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
    } else {
        QApplication::sendEvent(ui->lineEdit, event);
    }
    d->ignoreEvents = false;
}

void HorizontalSpinBox::keyReleaseEvent(QKeyEvent*event)
{
    if (d->ignoreEvents) return;

    d->ignoreEvents = true;
    if (event->key() == Qt::Key_Left) {

    } else if (event->key() == Qt::Key_Right) {

    } else if (event->key() == Qt::Key_Up) {

    } else if (event->key() == Qt::Key_Down) {

    } else if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
    } else {
        QApplication::sendEvent(ui->lineEdit, event);
    }
    d->ignoreEvents = false;
}

void HorizontalSpinBox::on_lineEdit_textChanged(const QString &arg1)
{
    this->setValue(arg1.toInt());
}
