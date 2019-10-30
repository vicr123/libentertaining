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
#include "loaddialog.h"
#include "ui_loaddialog.h"

#include <QShortcut>
#include <pauseoverlay.h>
#include "savesmodel.h"
#include "saveengine.h"
#include "musicengine.h"

struct LoadDialogPrivate {
    SavesModel* model;
    SaveObject selected;
};

LoadDialog::LoadDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadDialog)
{
    ui->setupUi(this);

    d = new LoadDialogPrivate();

    QPalette pal = ui->loadView->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->loadView->setPalette(pal);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Load"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, tr("Options"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        on_loadView_activated(ui->loadView->currentIndex());
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        emit rejected();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonX, [=] {

    });

    d->model = new SavesModel();
    d->model->setShowNewFile(false);
    ui->loadView->setModel(d->model);
    ui->loadView->setItemDelegate(new SavesDelegate(this));

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        ui->backButton->click();
    });
    connect(backShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->backButton->click();
    });

    this->setFocusProxy(ui->loadView);
}

LoadDialog::~LoadDialog()
{
    delete ui;
}

SaveObject LoadDialog::selectedSaveFile()
{
    return d->selected;
}

void LoadDialog::on_backButton_clicked()
{
    MusicEngine::playSoundEffect(MusicEngine::Backstep);
    emit rejected();
}

void LoadDialog::on_loadView_activated(const QModelIndex &index)
{
    MusicEngine::playSoundEffect(MusicEngine::Selection);

    //Load the data
    QString filename = d->model->data(index, Qt::UserRole).toString();
    SaveObject object = SaveEngine::getSaveByFilename(filename);
    d->selected = object;

    emit accepted();
}
