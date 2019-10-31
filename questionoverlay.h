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
#ifndef QUESTIONOVERLAY_H
#define QUESTIONOVERLAY_H

#include <QWidget>
#include <QMessageBox>
#include "libentertaining_global.h"

namespace Ui {
    class QuestionOverlay;
}

struct QuestionOverlayPrivate;
class LIBENTERTAINING_EXPORT QuestionOverlay : public QWidget
{
        Q_OBJECT

    public:
        explicit QuestionOverlay(QWidget *parent = nullptr);
        ~QuestionOverlay();

        void setTitle(QString title);
        void setText(QString text);
        void setButtons(QMessageBox::StandardButtons buttons, QString yesPrompt = "", bool isYesDestructive = false);
        void setIcon(QMessageBox::Icon icon);

    signals:
        void accepted(QMessageBox::StandardButton button);
        void rejected();

    private:
        Ui::QuestionOverlay *ui;
        QuestionOverlayPrivate* d;

        void reject();
};

#endif // QUESTIONOVERLAY_H
