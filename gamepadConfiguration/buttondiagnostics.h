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
#ifndef BUTTONDIAGNOSTICS_H
#define BUTTONDIAGNOSTICS_H

#include <QWidget>

namespace Ui {
    class ButtonDiagnostics;
}

struct ButtonDiagnosticsPrivate;
class ButtonDiagnostics : public QWidget
{
        Q_OBJECT

    public:
        explicit ButtonDiagnostics(int gamepadId, QWidget *parent = nullptr);
        ~ButtonDiagnostics();

        void quit();

    signals:
        void done();

    private slots:
        void on_finishButton_clicked();

    private:
        Ui::ButtonDiagnostics *ui;
        ButtonDiagnosticsPrivate* d;

        bool event(QEvent* event);
        void updateLabel();
};

#endif // BUTTONDIAGNOSTICS_H
