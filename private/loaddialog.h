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
#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include <QWidget>

namespace Ui {
    class LoadDialog;
}

struct LoadDialogPrivate;
struct SaveObject;
class LoadDialog : public QWidget
{
        Q_OBJECT

    public:
        explicit LoadDialog(QWidget *parent = nullptr);
        ~LoadDialog();

        SaveObject selectedSaveFile();

    private slots:
        void on_backButton_clicked();

        void on_loadView_activated(const QModelIndex &index);

    signals:
        void rejected();
        void accepted();

    private:
        Ui::LoadDialog *ui;
        LoadDialogPrivate* d;
};

#endif // LOADDIALOG_H
