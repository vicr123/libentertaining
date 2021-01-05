/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#ifndef BUTTONMAPPING_H
#define BUTTONMAPPING_H

#include <QWidget>

namespace Ui {
    class ButtonMapping;
}

struct ButtonMappingPrivate;
class ButtonMapping : public QWidget {
        Q_OBJECT

    public:
        explicit ButtonMapping(int gamepadId, QWidget* parent = nullptr);
        ~ButtonMapping();

    private slots:
        void on_titleLabel_backButtonClicked();

        void quit();

        void on_resetButton_clicked();

    signals:
        void done();

    private:
        Ui::ButtonMapping* ui;
        ButtonMappingPrivate* d;
};

#endif // BUTTONMAPPING_H
