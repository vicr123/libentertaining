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
#include "saveoverlay.h"

#include "pauseoverlay.h"
#include "private/savedialog.h"
#include <QEventLoop>
#include <QApplication>
#include "private/saveengine.h"

struct SaveOverlayPrivate {
    QWidget* parent;
    PauseOverlay* overlay;
};

SaveOverlay::SaveOverlay(QWidget *parent, PauseOverlay* pauseOverlay) : QObject(parent)
{
    d = new SaveOverlayPrivate();
    d->parent = parent;

    if (pauseOverlay == nullptr) {
        d->overlay = new PauseOverlay(parent, nullptr);
        connect(this, &SaveOverlay::destroyed, d->overlay, &PauseOverlay::deleteLater);
    } else {
        d->overlay = pauseOverlay;
    }
}

SaveOverlay::~SaveOverlay()
{
    delete d;
}

void SaveOverlay::save()
{
    QEventLoop* loop = new QEventLoop();

    SaveDialog* dlg = new SaveDialog(d->overlay);
    connect(dlg, &SaveDialog::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
    connect(dlg, &SaveDialog::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));

    d->overlay->pushOverlayWidget(dlg);

    if (loop->exec() == 0) {
        d->overlay->popOverlayWidget();

        SaveObject object = dlg->selectedSaveFile();

        QIODevice* device = object.getStream();

        QDataStream stream(device);
        stream << SAVE_FILE_MAGIC_NUMBER << stream.version();
        stream << QApplication::applicationName();

        QVariantMap metadata;
        emit provideMetadata(&metadata);
        stream << metadata;

        emit provideSaveData(&stream);

        device->deleteLater();
    } else {
        //User cancelled
        d->overlay->popOverlayWidget();
        emit canceled();
    }
}
