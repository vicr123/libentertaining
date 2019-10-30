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
#include "loadoverlay.h"

#include "pauseoverlay.h"
#include "private/loaddialog.h"
#include <QEventLoop>
#include <QApplication>
#include "private/saveengine.h"

struct LoadOverlayPrivate {
    QWidget* parent;
};

LoadOverlay::LoadOverlay(QWidget *parent) : QObject(parent)
{
    d = new LoadOverlayPrivate();
    d->parent = parent;
}

LoadOverlay::~LoadOverlay()
{
    delete d;
}

void LoadOverlay::load()
{
    QEventLoop* loop = new QEventLoop();

    LoadDialog* dlg = new LoadDialog();
    connect(dlg, &LoadDialog::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
    connect(dlg, &LoadDialog::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));

    PauseOverlay::overlayForWindow(d->parent)->pushOverlayWidget(dlg);

    if (loop->exec() == 0) {
        PauseOverlay::overlayForWindow(d->parent)->popOverlayWidget();

        SaveObject object = dlg->selectedSaveFile();

        QIODevice* device = object.getStream();

        QDataStream stream(device);
        uint magicNumber;
        int version;
        stream >> magicNumber >> version;

        if (magicNumber != SAVE_FILE_MAGIC_NUMBER) {
            //Error error!
            return;
        }

        stream.setVersion(version);

        QString appName;
        stream >> appName;

        QVariantMap metadata;
        stream >> metadata;

        emit loadData(&stream);

        device->deleteLater();
    } else {
        //User cancelled
        PauseOverlay::overlayForWindow(d->parent)->popOverlayWidget();
        emit canceled();
    }
}
