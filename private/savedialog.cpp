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
#include "savedialog.h"
#include "ui_savedialog.h"

#include <QShortcut>
#include "musicengine.h"
#include "savesmodel.h"
#include "saveengine.h"
#include "textinputoverlay.h"

struct SaveDialogPrivate {
    PauseOverlay* overlay;
    SavesModel* model;
    SaveObject selected;
};

SaveDialog::SaveDialog(PauseOverlay*overlay, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SaveDialog)
{
    ui->setupUi(this);

    d = new SaveDialogPrivate();
    d->overlay = overlay;

    QPalette pal = ui->saveView->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->saveView->setPalette(pal);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Save"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, tr("Create New Save"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        on_saveView_activated(ui->saveView->currentIndex());
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        emit rejected();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonX, [=] {
        this->newSave();
    });

    d->model = new SavesModel();
    d->model->setShowNewFile(true);
    ui->saveView->setModel(d->model);

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->backButton->click();
    });

    this->setFocusProxy(ui->saveView);
}

SaveDialog::~SaveDialog()
{
    delete d;
    delete ui;
}

SaveObject SaveDialog::selectedSaveFile()
{
    return d->selected;
}

void SaveDialog::on_backButton_clicked()
{
    MusicEngine::playSoundEffect(MusicEngine::Backstep);
    emit rejected();
}

void SaveDialog::newSave()
{
    MusicEngine::playSoundEffect(MusicEngine::Selection);

    bool canceled;
    QString filename = TextInputOverlay::getText(this, tr("What do you want to call this save?"), &canceled, "", QLineEdit::Normal, d->overlay);

    if (!canceled) {
        d->selected = SaveEngine::getSaveByFilename(filename);
        emit accepted();
    }
}

void SaveDialog::on_saveView_activated(const QModelIndex &index)
{
    //Save the data
    QVariant filename = d->model->data(index, Qt::UserRole);
    if (filename.type() == QVariant::Bool) {
        //This is the New Save option
        newSave();
    } else {
        SaveObject object = SaveEngine::getSaveByFilename(filename.toString());
        d->selected = object;

        emit accepted();
    }
}
