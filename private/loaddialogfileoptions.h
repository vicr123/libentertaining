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
#ifndef LOADDIALOGFILEOPTIONS_H
#define LOADDIALOGFILEOPTIONS_H

#include <QWidget>

namespace Ui {
    class LoadDialogFileOptions;
}

class LoadDialogFileOptions : public QWidget
{
        Q_OBJECT

    public:
        enum Action {
            Copy,
            Rename,
            Delete
        };

        explicit LoadDialogFileOptions(QWidget *parent = nullptr);
        ~LoadDialogFileOptions();

    signals:
        void performAction(Action action);
        void rejected();

    private slots:
        void on_backButton_clicked();

        void on_optionsWidget_activated(const QModelIndex &index);

    private:
        Ui::LoadDialogFileOptions *ui;
};

#endif // LOADDIALOGFILEOPTIONS_H
