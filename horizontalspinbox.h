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
#ifndef HORIZONTALSPINBOX_H
#define HORIZONTALSPINBOX_H

#include <QWidget>
#include "libentertaining_global.h"

namespace Ui {
    class HorizontalSpinBox;
}

struct HorizontalSpinBoxPrivate;
class LIBENTERTAINING_EXPORT HorizontalSpinBox : public QWidget
{
        Q_OBJECT

    public:
        explicit HorizontalSpinBox(QWidget *parent = nullptr);
        ~HorizontalSpinBox();

        void setValue(int value);
        int value();

        int max();
        int min();
        void setRange(int min, int max);

    private slots:
        void on_upButton_clicked();

        void on_downButton_clicked();

        void on_lineEdit_textChanged(const QString &arg1);

    signals:
        void valueChanged(int value);

    private:
        Ui::HorizontalSpinBox *ui;
        HorizontalSpinBoxPrivate* d;

        void keyPressEvent(QKeyEvent* event);
        void keyReleaseEvent(QKeyEvent* event);
};

#endif // HORIZONTALSPINBOX_H
