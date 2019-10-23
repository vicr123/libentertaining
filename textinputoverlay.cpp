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
#include "textinputoverlay.h"
#include "ui_textinputoverlay.h"

#include <the-libs_global.h>
#include <QEventLoop>
#include <QPainter>
#include <QShortcut>
#include <QKeyEvent>
#include "musicengine.h"
#include "pauseoverlay.h"

#include "keyboards/uskeyboard.h"

struct TextInputOverlayPrivate {
    PauseOverlay* overlay;
    QWidget* parent;

    QList<Keyboard*> keyboards;
};

TextInputOverlay::TextInputOverlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextInputOverlay)
{
    ui->setupUi(this);

    d = new TextInputOverlayPrivate();
    d->parent = parent;
    d->overlay = new PauseOverlay(this);

    ui->keyboardWidget->setFixedHeight(SC_DPI(200));

    QPalette pal = ui->responseBox->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->responseBox->setPalette(pal);

    QShortcut* cancelShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(cancelShortcut, &QShortcut::activated, this, [=] {
        ui->cancelButton->click();
    });
    connect(cancelShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->cancelButton->click();
    });

    connect(ui->keyboardWidget, &Keyboard::typeKey, this, [=](QString key) {
        QString text = ui->responseBox->text();
        text.append(key);
        ui->responseBox->setText(text);
    });
    connect(ui->keyboardWidget, &Keyboard::backspace, this, [=] {
        QString text = ui->responseBox->text();
        text.chop(1);
        ui->responseBox->setText(text);
    });
    connect(ui->keyboardWidget, &Keyboard::accept, this, [=] {
        emit accepted(ui->responseBox->text());
    });
    connect(ui->keyboardWidget, &Keyboard::replayKeyEvent, this, [=](QKeyEvent* event) {
        QApplication::sendEvent(ui->responseBox, event);
        ui->responseBox->setFocus();
    });

    //Set up keyboard layouts
    KeyboardLayout usLayout;
    usLayout.name = "en-US";
    usLayout.keys = {
        {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', KeyboardKey::Backspace},
        {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', KeyboardKey::Ok},
        {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', KeyboardKey::Shift}
    };
    ui->keyboardWidget->setCurrentLayout(usLayout);

    ui->responseBox->installEventFilter(this);
}

TextInputOverlay::~TextInputOverlay()
{
    d->overlay->deleteLater();
    delete ui;
    delete d;
}

QString TextInputOverlay::getText(QWidget* parent, QString question, bool*canceled)
{
    QEventLoop* loop = new QEventLoop();

    TextInputOverlay* input = new TextInputOverlay(parent);
    input->setQuestion(question);
    input->show();
    connect(input, &TextInputOverlay::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
    connect(input, &TextInputOverlay::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));
    if (loop->exec() == 0) {
        if (canceled != nullptr) *canceled = false;
        QString response = input->response();
        input->deleteLater();
        loop->deleteLater();
        return response;
    } else {
        if (canceled != nullptr) *canceled = true;
        input->deleteLater();
        loop->deleteLater();
        return "";
    }
}

void TextInputOverlay::setQuestion(QString question)
{
    if (question.isEmpty()) {
        ui->questionLabel->setVisible(false);
    } else {
        ui->questionLabel->setVisible(true);
        ui->questionLabel->setText(question);
    }
}

QString TextInputOverlay::response()
{
    return ui->responseBox->text();
}

void TextInputOverlay::show()
{
    d->overlay->showOverlay(d->parent);
    ui->responseBox->setFocus();
}

void TextInputOverlay::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 127));
    painter.setPen(Qt::transparent);
    painter.drawRect(0, 0, this->width(), this->height());
}

bool TextInputOverlay::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == ui->responseBox && event->type() == QKeyEvent::KeyPress) {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Left || e->key() == Qt::Key_Up || e->key() == Qt::Key_Right) {
            ui->keyboardWidget->setFocus();
        }
    }
    return false;
}

void TextInputOverlay::on_responseBox_returnPressed()
{

}

void TextInputOverlay::on_okButton_clicked()
{
    MusicEngine::playSoundEffect(MusicEngine::Selection);
    emit accepted(ui->responseBox->text());
}

void TextInputOverlay::on_cancelButton_clicked()
{
    MusicEngine::playSoundEffect(MusicEngine::Backstep);
    emit rejected();
}
