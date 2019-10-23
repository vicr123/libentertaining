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
#ifndef DIALOGUEOVERLAY_H
#define DIALOGUEOVERLAY_H

#include <QWidget>
#include <QMap>

namespace Ui {
    class DialogueOverlay;
}

struct DialogueOverlayPrivate;
class DialogueOverlay : public QWidget
{
        Q_OBJECT

    public:
        explicit DialogueOverlay(QWidget *parent);
        ~DialogueOverlay();

    public slots:
        void setNewDialogue(QString dialogue, QMap<QString, QString> options = QMap<QString, QString>());
        void setMultiDialogue(QStringList dialogues, QMap<QString, QString> optionsForLastDialogue = QMap<QString, QString>());
        void dismiss();

    signals:
        void progressDialogue(QString option);

    private:
        Ui::DialogueOverlay *ui;
        DialogueOverlayPrivate* d;

        void progressDialogue();

        void showFrame(bool show);
        void progressText();
        void skipTextAnimation();

        void keyReleaseEvent(QKeyEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // DIALOGUEOVERLAY_H
