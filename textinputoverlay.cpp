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
#include <QValidator>
#include <terrorflash.h>
#include "pauseoverlay.h"
#include "musicengine.h"
#include "private/textinputlineedithandler.h"

#include "keyboards/uskeyboard.h"
#include "keyboards/keyboardlayoutsdatabase.h"

struct TextInputOverlayPrivate {
    QWidget* parent;

    QList<Keyboard*> keyboards;
    QValidator* validator = nullptr;
    QString validatorError;

    static QList<TextInputLineEditHandler*> handledLineEdits;
};

QList<TextInputLineEditHandler*> TextInputOverlayPrivate::handledLineEdits = QList<TextInputLineEditHandler*>();

TextInputOverlay::TextInputOverlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextInputOverlay)
{
    ui->setupUi(this);

    d = new TextInputOverlayPrivate();
    d->parent = parent;

    ui->keyboardWidget->setFixedHeight(SC_DPI(300));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        QString text = ui->responseBox->text();
        if (text.isEmpty()) {
            //Cancel instead
            ui->cancelButton->click();
        } else {
            MusicEngine::playSoundEffect(MusicEngine::Backstep);
            text.chop(1);
            ui->responseBox->setText(text);
        }
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [=] {
        tryAccept();
    });
    ui->gamepadHud->bindKey(Qt::Key_Space, QGamepadManager::ButtonX);
    ui->gamepadHud->bindKey(Qt::Key_Shift, QGamepadManager::ButtonY);

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
        ui->responseBox->insert(key);
    });
    connect(ui->keyboardWidget, &Keyboard::backspace, this, [=] {
        ui->responseBox->backspace();
    });
    connect(ui->keyboardWidget, &Keyboard::accept, this, [=] {
        tryAccept();
    });
    connect(ui->keyboardWidget, &Keyboard::replayKeyEvent, this, [=](QKeyEvent* event) {
        QApplication::sendEvent(ui->responseBox, event);
        ui->responseBox->setFocus();
    });
    connect(ui->keyboardWidget, &Keyboard::capsStateChanged, this, [=](Keyboard::CapsState capsState) {
        switch (capsState) {
            case Keyboard::None:
            case Keyboard::Shift:
                ui->gamepadHud->setButtonText(QGamepadManager::ButtonY, tr("Shift"));
                break;
            case Keyboard::Caps:
                ui->gamepadHud->setButtonText(QGamepadManager::ButtonY, tr("Caps Off"));
                break;

        }
    });
    connect(ui->keyboardWidget, &Keyboard::keyboardUpdated, this, [=] {
        ui->gamepadHud->removeText(QGamepadManager::ButtonA);
        ui->gamepadHud->removeText(QGamepadManager::ButtonB);
        ui->gamepadHud->removeText(QGamepadManager::ButtonY);
        ui->gamepadHud->removeText(QGamepadManager::ButtonX);
        ui->gamepadHud->removeText(QGamepadManager::ButtonStart);

        ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Key"));
        if (ui->responseBox->text().isEmpty()) {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Cancel"));
        } else {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Backspace"));
        }

        if (ui->keyboardWidget->canShift()) {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonY, tr("Shift"));
            ui->gamepadHud->setButtonAction(QGamepadManager::ButtonY, std::bind(&TextInputOverlay::keyboardShift, this));
        } else {
            ui->gamepadHud->removeButtonAction(QGamepadManager::ButtonY);
        }

        if (ui->keyboardWidget->canSpace()) {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, tr("Space"));
            ui->gamepadHud->setButtonAction(QGamepadManager::ButtonX, std::bind(&TextInputOverlay::keyboardSpace, this));
        } else {
            ui->gamepadHud->removeButtonAction(QGamepadManager::ButtonX);
        }

        ui->gamepadHud->setButtonText(QGamepadManager::ButtonStart, tr("OK"));
    });

    ui->keyboardWidget->setCurrentLayout(KeyboardLayoutsDatabase::layoutForName("en-US"));

    this->setFocusProxy(ui->responseBox);
    ui->responseBox->installEventFilter(this);
}

TextInputOverlay::~TextInputOverlay()
{
    delete ui;
    delete d;
}

QString TextInputOverlay::getText(QWidget* parent, QString question, bool*canceled, QString defaultText, QLineEdit::EchoMode echoMode)
{
    QEventLoop* loop = new QEventLoop();

    TextInputOverlay* input = new TextInputOverlay(parent);
    input->setQuestion(question);
    input->setResponse(defaultText);
    input->setEchoMode(echoMode);
    input->show();
    connect(input, &TextInputOverlay::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
    connect(input, &TextInputOverlay::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));
    if (loop->exec() == 0) {
        if (canceled != nullptr) *canceled = false;
        input->hide();
        QString response = input->response();
        input->deleteLater();
        loop->deleteLater();
        return response;
    } else {
        if (canceled != nullptr) *canceled = true;
        input->hide();
        input->deleteLater();
        loop->deleteLater();
        return "";
    }
}

int TextInputOverlay::getInt(QWidget*parent, QString question, bool*canceled, int defaultText, int min, int max, QLineEdit::EchoMode echoMode)
{
    QEventLoop* loop = new QEventLoop();

    QIntValidator validator(min, max);

    TextInputOverlay* input = new TextInputOverlay(parent);
    input->setQuestion(question);
    input->setResponse(QString::number(defaultText));
    input->setEchoMode(echoMode);
    input->setInputMethodHints(Qt::ImhDigitsOnly | Qt::ImhPreferNumbers);
    input->setValidator(&validator, tr("Enter a number between %1 and %2").arg(min).arg(max));
    input->show();
    connect(input, &TextInputOverlay::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
    connect(input, &TextInputOverlay::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));
    if (loop->exec() == 0) {
        if (canceled != nullptr) *canceled = false;
        input->hide();
        QString response = input->response();
        input->deleteLater();
        loop->deleteLater();
        return response.toInt();
    } else {
        if (canceled != nullptr) *canceled = true;
        input->hide();
        input->deleteLater();
        loop->deleteLater();
        return 0;
    }
}

QString TextInputOverlay::getTextWithRegex(QWidget*parent, QString question, QRegularExpression regex, bool*canceled, QString defaultText, QString errorText, Qt::InputMethodHints hints, QLineEdit::EchoMode echoMode)
{
    QEventLoop* loop = new QEventLoop();

    QRegularExpressionValidator validator(regex);

    TextInputOverlay* input = new TextInputOverlay(parent);
    input->setQuestion(question);
    input->setResponse(defaultText);
    input->setEchoMode(echoMode);
    input->setInputMethodHints(hints);
    input->setValidator(&validator, errorText);
    input->show();
    connect(input, &TextInputOverlay::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
    connect(input, &TextInputOverlay::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));
    if (loop->exec() == 0) {
        if (canceled != nullptr) *canceled = false;
        input->hide();
        QString response = input->response();
        input->deleteLater();
        loop->deleteLater();
        return response;
    } else {
        if (canceled != nullptr) *canceled = true;
        input->hide();
        input->deleteLater();
        loop->deleteLater();
        return "";
    }
}

void TextInputOverlay::installHandler(QLineEdit* lineEdit, QString question, QWidget*overlayOn)
{
    TextInputLineEditHandler* handler = new TextInputLineEditHandler(lineEdit);
    TextInputOverlayPrivate::handledLineEdits.append(handler);
    connect(handler, &TextInputLineEditHandler::destroyed, [=] {
        TextInputOverlayPrivate::handledLineEdits.removeAll(handler);
    });
    connect(handler, &TextInputLineEditHandler::openKeyboard, [=] {
        QEventLoop* loop = new QEventLoop();

        QWidget* overlay = overlayOn;
        if (overlay == nullptr) overlay = lineEdit->parentWidget();

        QString q = question;
        if (q.isEmpty()) q = lineEdit->placeholderText();

        TextInputOverlay* input = new TextInputOverlay(overlay);
        input->setQuestion(question);
        input->setResponse(lineEdit->text());
        input->setEchoMode(lineEdit->echoMode());
        input->setInputMethodHints(lineEdit->inputMethodHints());
        input->show();
        connect(input, &TextInputOverlay::accepted, loop, std::bind(&QEventLoop::exit, loop, 0));
        connect(input, &TextInputOverlay::rejected, loop, std::bind(&QEventLoop::exit, loop, 1));
        if (loop->exec() == 0) {
            lineEdit->setText(input->response());
        }
        lineEdit->setFocus();
    });
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

void TextInputOverlay::setResponse(QString response)
{
    ui->responseBox->setText(response);
}

QString TextInputOverlay::response()
{
    return ui->responseBox->text();
}

void TextInputOverlay::setEchoMode(QLineEdit::EchoMode echoMode)
{
    ui->responseBox->setEchoMode(echoMode);
}

void TextInputOverlay::setValidator(QValidator*validator, QString errorMessage)
{
    d->validator = validator;
    d->validatorError = errorMessage;
}

void TextInputOverlay::show()
{
    //Choose the correct layout
    KeyboardLayout layout = KeyboardLayoutsDatabase::layoutForName("en-US");
    if (this->inputMethodHints() & Qt::ImhDigitsOnly) layout = KeyboardLayoutsDatabase::layoutForName("numOnly");
    ui->keyboardWidget->setCurrentLayout(layout);

    ui->responseBox->selectAll();

    PauseOverlay::overlayForWindow(d->parent)->pushOverlayWidget(this);

    #ifdef Q_OS_ANDROID
        //Use the built in system keyboard on Android
        ui->keyboardWidget->setVisible(false);
        QApplication::inputMethod()->show();
    #endif
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
        if (e->key() == Qt::Key_Down) {
            ui->keyboardWidget->setFocus();
            return true;
        }
    }
    return false;
}

void TextInputOverlay::on_responseBox_returnPressed()
{

}

void TextInputOverlay::on_okButton_clicked()
{
    tryAccept();
}

void TextInputOverlay::on_cancelButton_clicked()
{
    MusicEngine::playSoundEffect(MusicEngine::Backstep);
    PauseOverlay::overlayForWindow(d->parent)->popOverlayWidget([=] {
        emit rejected();
    });
}

void TextInputOverlay::on_responseBox_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Cancel"));
    } else {
        ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Backspace"));
    }
}

void TextInputOverlay::tryAccept()
{
    QString text = ui->responseBox->text();
    QString error = "";

    if (d->validator != nullptr) {
        int pos = 0;
        QValidator::State state = d->validator->validate(text, pos);
        switch (state) {
            case QValidator::Invalid:
            case QValidator::Intermediate:
                error = d->validatorError;
                break;
            case QValidator::Acceptable:
                break;
        }
    }

    if (error.isEmpty()) {
        MusicEngine::playSoundEffect(MusicEngine::Selection);
        PauseOverlay::overlayForWindow(d->parent)->popOverlayWidget([=] {
            emit accepted(ui->responseBox->text());
        });
    } else {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->errorLabel->setText(error);
        tErrorFlash::flashError(ui->responseBox);
    }
}

void TextInputOverlay::keyboardShift()
{
    MusicEngine::playSoundEffect(MusicEngine::Selection);
    if (ui->keyboardWidget->capsState() == Keyboard::None) {
        ui->keyboardWidget->setCapsState(Keyboard::Shift);
    } else {
        ui->keyboardWidget->setCapsState(Keyboard::None);
    }
}

void TextInputOverlay::keyboardSpace()
{
    MusicEngine::playSoundEffect(MusicEngine::Selection);
    QString text = ui->responseBox->text();
    text.append(" ");
    ui->responseBox->setText(text);
}
