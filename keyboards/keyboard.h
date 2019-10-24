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
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <functional>

struct KeyboardPrivate;

class QPushButton;
struct KeyboardKey {
    enum KnownKeys {
        Space,
        Backspace,
        Shift,
        Ok,
        SetNumeric,
        SetLayout,

        Dummy
    };

    KeyboardKey(const char character);
    KeyboardKey(const QChar character);
    KeyboardKey(std::function<void()> clickHandler, QString label);
    KeyboardKey(KnownKeys knownKey);

    int type;

    QChar character;

    QString label;
    std::function<void()> clickHandler;

    KnownKeys knownKey;
};

struct KeyboardLayout {
    QVector<QVector<KeyboardKey>> keys;

    bool honourShift = true;
    QString name;
};

class Keyboard : public QWidget {
        Q_OBJECT

    public:
        enum CapsState {
            None,
            Shift,
            Caps
        };

        explicit Keyboard(QWidget* parent = nullptr);
        ~Keyboard();

        void setCurrentLayout(KeyboardLayout layout);

        void setCapsState(CapsState capsState);
        CapsState capsState();

    signals:
        void typeKey(QString key);
        void backspace();
        void accept();
        void replayKeyEvent(QKeyEvent* event);
        void capsStateChanged(CapsState capsState);

        void updateKeys();

    private:
        KeyboardPrivate* d;

        void updateButton(QPushButton* button, KeyboardKey key);
        void setCurrentButton(QPushButton* button);
        void tryMoveFocus(QPoint pos);


        void resizeEvent(QResizeEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // KEYBOARD_H

