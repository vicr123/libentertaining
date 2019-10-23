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

#include "keyboard.h"
#include <QBoxLayout>
#include <QMap>
#include <QStackedWidget>
#include <QPushButton>
#include <QKeyEvent>
#include <focuspointer.h>
#include <the-libs_global.h>
#include "musicengine.h"

struct KeyboardPrivate {
    KeyboardLayout currentLayout;

    QBoxLayout* holderLayout;
    QWidget* currentWidget = nullptr;

    QList<QPushButton*> buttons;
    QWidget* currentButton = nullptr;

    Keyboard::CapsState capsState = Keyboard::None;
};

Keyboard::Keyboard(QWidget* parent) : QWidget(parent)
{
    d = new KeyboardPrivate();

    this->setAutoFillBackground(true);

    QWidget* holder = new QWidget(this);
    d->holderLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    d->holderLayout->setContentsMargins(0, 0, 0, 0);
    d->holderLayout->setSpacing(0);
    holder->setLayout(d->holderLayout);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);
    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->addWidget(holder);
    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    this->setLayout(layout);

    QFont font = this->font();
    font.setPointSize(15);
    this->setFont(font);
}

Keyboard::~Keyboard()
{
    delete d;
}

void Keyboard::setCurrentLayout(KeyboardLayout layout)
{
    if (d->currentWidget != nullptr) {
        d->holderLayout->removeWidget(d->currentWidget);
        d->currentWidget->deleteLater();
    }
    d->buttons.clear();

    d->currentWidget = new QWidget();
    QBoxLayout* mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    d->currentWidget->setLayout(mainLayout);

    d->currentLayout = layout;
    for (QVector<KeyboardKey> keys : layout.keys) {
        QBoxLayout* l = new QBoxLayout(QBoxLayout::LeftToRight);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);

        l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

        for (KeyboardKey key : keys) {
            QPushButton* button = new QPushButton(d->currentWidget);
            button->setProperty("keyboard_isKeyboardButton", true);
            button->installEventFilter(this);
            button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            connect(this, &Keyboard::updateKeys, button, std::bind(&Keyboard::updateButton, this, button, key));
            updateButton(button, key);
            d->buttons.append(button);
            l->addWidget(button);
        }

        l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

        mainLayout->addLayout(l);
    }

    d->holderLayout->addWidget(d->currentWidget);

    setCurrentButton(d->buttons.first());
}

void Keyboard::setCapsState(Keyboard::CapsState capsState)
{
    d->capsState = capsState;
    emit updateKeys();
    emit capsStateChanged(capsState);
}

Keyboard::CapsState Keyboard::capsState()
{
    return d->capsState;
}

void Keyboard::updateButton(QPushButton*button, KeyboardKey key)
{
    disconnect(button, &QPushButton::clicked, nullptr, nullptr);
    button->setFlat(true);

    int standardWidth = SC_DPI(70);

    switch (key.type) {
        case 0: { //Key
            QChar charToSet;

            if (d->currentLayout.honourShift) {
                if (d->capsState == None) {
                    charToSet = key.character.toLower();
                } else {
                    charToSet = key.character.toUpper();
                }
            } else {
                charToSet = key.character;
            }

            button->setText(charToSet);
            button->setFixedWidth(standardWidth);
            connect(button, &QPushButton::clicked, this, [=] {
                emit typeKey(charToSet);

                if (d->capsState == Shift) setCapsState(None);
                MusicEngine::playSoundEffect(MusicEngine::Selection);
            });
            break;
        }
        case 1: //Code
            button->setText(key.label);
            connect(button, &QPushButton::clicked, this, [=] {
                key.clickHandler();
            });
            break;
        case 2: //Known Key
            switch (key.knownKey) {
                case KeyboardKey::Backspace:
                    button->setText(tr("BKSP"));
                    button->setFixedWidth(standardWidth);
                    connect(button, &QPushButton::clicked, this, [=] {
                        emit backspace();
                        MusicEngine::playSoundEffect(MusicEngine::Backstep);
                    });
                    break;
                case KeyboardKey::Shift:
                    button->setText(tr("SHF"));
                    button->setFixedWidth(standardWidth);
                    connect(button, &QPushButton::clicked, this, [=] {
                        MusicEngine::playSoundEffect(MusicEngine::Selection);
                        if (d->capsState == None) {
                            setCapsState(Shift);
                        } else {
                            setCapsState(None);
                        }
                    });
                    break;
                case KeyboardKey::Ok:
                    button->setFlat(false);
                    button->setText(tr("OK"));
                    button->setFixedWidth(standardWidth);
                    connect(button, &QPushButton::clicked, this, [=] {
                        MusicEngine::playSoundEffect(MusicEngine::Selection);
                        emit accept();
                    });
                    break;
                case KeyboardKey::Space:
                    button->setFlat(true);
                    button->setText("_");
                    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                    connect(button, &QPushButton::clicked, this, [=] {
                        MusicEngine::playSoundEffect(MusicEngine::Selection);
                        emit typeKey(" ");
                    });
                    break;
                case KeyboardKey::SetNumeric:
                    button->setFlat(true);
                    button->setText("123");
                    button->setFixedWidth(standardWidth);
                    connect(button, &QPushButton::clicked, this, [=] {
                        MusicEngine::playSoundEffect(MusicEngine::Selection);
                        //TODO: Change to numeric and symbol keyboard
                    });
                    break;
                case KeyboardKey::SetLayout:
                    button->setFlat(true);
                    button->setText("Hè");
                    button->setFixedWidth(standardWidth);
                    connect(button, &QPushButton::clicked, this, [=] {
                        MusicEngine::playSoundEffect(MusicEngine::Selection);
                        //TODO: Change to another keyboard
                    });
                    break;
            }
            break;
    }
}

void Keyboard::setCurrentButton(QPushButton*button)
{
    d->currentButton = button;
    button->setFocus();
    this->setFocusProxy(button);
}

void Keyboard::tryMoveFocus(QPoint pos)
{
    QRect thisGeometry(this->mapToGlobal(QPoint(0, 0)), this->size());
    if (!thisGeometry.contains(pos)) {
        return; //This point isn't on the keyboard
    }

    QPoint childPos = this->mapFromGlobal(pos);
    QWidget* currentWidget = this;
    while (currentWidget->childAt(childPos) != nullptr) {
        currentWidget = currentWidget->childAt(childPos);
        childPos = currentWidget->mapFromGlobal(pos);
    }

    if (currentWidget->property("keyboard_isKeyboardButton").toBool()) {
        setCurrentButton(static_cast<QPushButton*>(currentWidget));
    }
}

void Keyboard::resizeEvent(QResizeEvent* event)
{
    emit updateKeys();
}

bool Keyboard::eventFilter(QObject*watched, QEvent*event)
{
    if (watched->property("keyboard_isKeyboardButton").toBool()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* e = static_cast<QKeyEvent*>(event);
            QPushButton* button = static_cast<QPushButton*>(watched);

            switch (e->key()) {
                case Qt::Key_Up:
                    tryMoveFocus(button->mapToGlobal(QPoint(0, -10)));
                    break;
                case Qt::Key_Left:
                    tryMoveFocus(button->mapToGlobal(QPoint(-10, 0)));
                    break;
                case Qt::Key_Right:
                    tryMoveFocus(button->mapToGlobal(QPoint(button->width() + 10, 0)));
                    break;
                case Qt::Key_Down:
                    tryMoveFocus(button->mapToGlobal(QPoint(0, button->height() + 10)));
                    break;
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    button->click();
                    break;
                default:
                    emit replayKeyEvent(e);
            }

            return true;
        }
    }
    return false;
}

KeyboardKey::KeyboardKey(char character)
{
    this->type = 0;
    this->character = character;
}

KeyboardKey::KeyboardKey(QChar character)
{
    this->type = 0;
    this->character = character;
}

KeyboardKey::KeyboardKey(std::function<void ()> clickHandler, QString label)
{
    this->type = 1;
    this->clickHandler = clickHandler;
    this->label = label;
}

KeyboardKey::KeyboardKey(KeyboardKey::KnownKeys knownKey)
{
    this->type = 2;
    this->knownKey = knownKey;
}
