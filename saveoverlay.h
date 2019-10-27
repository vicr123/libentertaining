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
#ifndef SAVEOVERLAY_H
#define SAVEOVERLAY_H

#include "libentertaining_global.h"
#include <QObject>

class PauseOverlay;
struct SaveOverlayPrivate;
class SaveOverlay : public QObject
{
        Q_OBJECT
    public:
        explicit SaveOverlay(QWidget *parent = nullptr, PauseOverlay* pauseOverlay = nullptr);
        ~SaveOverlay();

        void save();

    signals:
        void provideMetadata(QVariantMap* metadata);
        void provideSaveData(QDataStream* stream);
        void saved();
        void canceled();

    public slots:

    private:
        SaveOverlayPrivate* d;
};

#endif // SAVEOVERLAY_H
