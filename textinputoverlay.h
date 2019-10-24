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
#ifndef TEXTINPUTOVERLAY_H
#define TEXTINPUTOVERLAY_H

#include "libentertaining_global.h"
#include <QWidget>
#include <QLineEdit>

namespace Ui {
    class TextInputOverlay;
}

struct TextInputOverlayPrivate;
class LIBENTERTAINING_EXPORT TextInputOverlay : public QWidget
{
        Q_OBJECT

    public:
        explicit TextInputOverlay(QWidget *parent);
        ~TextInputOverlay();

        static QString getText(QWidget* parent, QString question, bool* canceled = nullptr, QString defaultText = "", QLineEdit::EchoMode echoMode = QLineEdit::Normal);
        static int getInt(QWidget* parent, QString question, bool* canceled = nullptr, int defaultText = 0, QLineEdit::EchoMode echoMode = QLineEdit::Normal);
        static void installHandler(QLineEdit* lineEdit, QString question = "", QWidget* overlayOn = nullptr);

        void setQuestion(QString question);

        void setResponse(QString response);
        QString response();

        void setEchoMode(QLineEdit::EchoMode echoMode);

    public slots:
        void show();

    signals:
        void accepted(QString response);
        void rejected();

    private slots:
        void on_responseBox_returnPressed();

        void on_okButton_clicked();

        void on_cancelButton_clicked();

        void on_responseBox_textChanged(const QString &arg1);

        void tryAccept();

    private:
        Ui::TextInputOverlay *ui;

        TextInputOverlayPrivate* d;

        void paintEvent(QPaintEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // TEXTINPUTOVERLAY_H
