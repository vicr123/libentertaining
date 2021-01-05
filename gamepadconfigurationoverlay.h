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
#ifndef GAMEPADCONFIGURATIONOVERLAY_H
#define GAMEPADCONFIGURATIONOVERLAY_H

#include <QWidget>
#include <libentertaining_global.h>

namespace Ui {
    class GamepadConfigurationOverlay;
}

class LIBENTERTAINING_EXPORT GamepadConfigurationOverlay : public QWidget {
        Q_OBJECT

    public:
        explicit GamepadConfigurationOverlay(QWidget* parent = nullptr);
        ~GamepadConfigurationOverlay();

    private slots:
        void on_backButton_clicked();

        void on_checkButtons_clicked();

        void on_checkSticks_clicked();

        void on_configureButtonMappings_clicked();

        void on_gamepadIconTypeButton_clicked();

    signals:
        void done();

    private:
        Ui::GamepadConfigurationOverlay* ui;

        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // GAMEPADCONFIGURATIONOVERLAY_H
