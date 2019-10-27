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
#ifndef PAUSEOVERLAY_H
#define PAUSEOVERLAY_H

#include "libentertaining_global.h"
#include <QWidget>
#include <functional>

struct PauseOverlayPrivate;
class LIBENTERTAINING_EXPORT PauseOverlay : public QWidget
{
        Q_OBJECT
    public:
        explicit PauseOverlay(QWidget* overlayOver = nullptr, QWidget* overlayWidget = nullptr, QWidget *parent = nullptr);
        ~PauseOverlay();

        void setOverlayOver(QWidget* overlayOver);

        void showOverlay();
        void hideOverlay();

        void setOverlayWidget(QWidget* overlayWidget);
        void pushOverlayWidget(QWidget* overlayWidget);
        void popOverlayWidget(std::function<void()> after = []{});

        void setDeleteOnHide(bool deleteOnHide);

    signals:
        void widgetPopped();

    public slots:

    private:
        PauseOverlayPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);
        void paintEvent(QPaintEvent* event);

        void setNewOverlayWidget(QWidget* widget, std::function<void()> after = []{});
};

#endif // PAUSEOVERLAY_H
