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
#include "loaddialogfileoptions.h"
#include "ui_loaddialogfileoptions.h"

#include <QShortcut>
#include <the-libs_global.h>

LoadDialogFileOptions::LoadDialogFileOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadDialogFileOptions)
{
    ui->setupUi(this);

    ui->optionsWidget->setFixedWidth(SC_DPI(600));
    this->setFocusProxy(ui->optionsWidget);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        on_optionsWidget_activated(ui->optionsWidget->currentIndex());
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        ui->backButton->click();
    });
    connect(backShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->backButton->click();
    });
}

LoadDialogFileOptions::~LoadDialogFileOptions()
{
    delete ui;
}

void LoadDialogFileOptions::on_backButton_clicked()
{
    emit rejected();
}

void LoadDialogFileOptions::on_optionsWidget_activated(const QModelIndex &index)
{
    Action action;
    switch (index.row()) {
        case 0:
            action = Copy;
            break;
        case 1:
            action = Rename;
            break;
        case 2:
            action = Delete;
            break;
        default:
            return;
    }

    emit performAction(action);
}
