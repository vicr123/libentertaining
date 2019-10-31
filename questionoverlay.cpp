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
#include "questionoverlay.h"
#include "ui_questionoverlay.h"

#include <the-libs_global.h>
#include "pauseoverlay.h"
#include <QPushButton>
#include <QShortcut>
#include <QKeyEvent>
#include <musicengine.h>

struct QuestionOverlayPrivate {
    QMessageBox::Icon icon;
};

QuestionOverlay::QuestionOverlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QuestionOverlay)
{
    ui->setupUi(this);
    d = new QuestionOverlayPrivate();

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &event);

        QKeyEvent event2(QKeyEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &event2);
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        reject();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        reject();
    });
}

QuestionOverlay::~QuestionOverlay()
{
    delete d;
    delete ui;
}

void QuestionOverlay::setTitle(QString title)
{
    ui->titleLabel->setText(title);
}

void QuestionOverlay::setText(QString text)
{
    ui->messageLabel->setText(text);
}

void QuestionOverlay::setButtons(QMessageBox::StandardButtons buttons, QString yesPrompt, bool isYesDestructive)
{
    if (yesPrompt == "") yesPrompt = tr("Yes");

    QLayoutItem* item;
    while ((item = ui->buttonsLayout->takeAt(0)) != nullptr) {
        ui->buttonsLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    struct ButtonDef {
        ButtonDef() {}
        ButtonDef(QString text, QMessageBox::StandardButton id, QString icon = "") {
            this->text = text;
            this->id = id;
            this->icon = icon;
        }

        QString text;
        QMessageBox::StandardButton id;
        QString icon = "";
    };
    QList<ButtonDef> shownButtons;

    if (buttons & QMessageBox::Cancel) shownButtons.append(ButtonDef(tr("Cancel"), QMessageBox::Cancel, "go-previous"));
    if (buttons & QMessageBox::Discard) shownButtons.append(ButtonDef(tr("Discard"), QMessageBox::Discard, "user-trash"));
    if (buttons & QMessageBox::Open) shownButtons.append(ButtonDef(tr("Open"), QMessageBox::Open, "document-open"));
    if (buttons & QMessageBox::Save) shownButtons.append(ButtonDef(tr("Save"), QMessageBox::Save, "document-save"));
    if (buttons & QMessageBox::SaveAll) shownButtons.append(ButtonDef(tr("Save All"), QMessageBox::SaveAll, "document-save-all"));
    if (buttons & QMessageBox::Yes) shownButtons.append(ButtonDef(yesPrompt, QMessageBox::Yes, "dialog-ok"));
    if (buttons & QMessageBox::YesToAll) shownButtons.append(ButtonDef(tr("Yes To All"), QMessageBox::YesToAll, "dialog-ok"));
    if (buttons & QMessageBox::No) shownButtons.append(ButtonDef(tr("No"), QMessageBox::No, "dialog-cancel"));
    if (buttons & QMessageBox::NoToAll) shownButtons.append(ButtonDef(tr("No To All"), QMessageBox::NoToAll, "dialog-cancel"));
    if (buttons & QMessageBox::Abort) shownButtons.append(ButtonDef(tr("Abort"), QMessageBox::Abort, "dialog-close"));
    if (buttons & QMessageBox::Retry) shownButtons.append(ButtonDef(tr("Retry"), QMessageBox::Retry, "view-refresh"));
    if (buttons & QMessageBox::Ignore) shownButtons.append(ButtonDef(tr("Ignore"), QMessageBox::Ignore));
    if (buttons & QMessageBox::Close) shownButtons.append(ButtonDef(tr("Close"), QMessageBox::Close, "dialog-close"));
    if (buttons & QMessageBox::Ok) shownButtons.append(ButtonDef(tr("OK"), QMessageBox::Ok, "dialog-ok"));

    QPushButton* firstButton = nullptr;
    QPushButton* lastButton = nullptr;
    for (ButtonDef button : shownButtons) {
        QPushButton* b = new QPushButton(this);
        b->setText(button.text);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        if (button.id == QMessageBox::Yes && isYesDestructive) {
            b->setProperty("type", "destructive");
        }
        if (!button.icon.isEmpty()) b->setIcon(QIcon(QStringLiteral(":/libentertaining/icons/%1.svg").arg(button.icon)));
        connect(b, &QPushButton::clicked, this, [=] {
            PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
                emit accepted(button.id);
            });
        });
        ui->buttonsLayout->addWidget(b);

        if (firstButton == nullptr) firstButton = b;
        lastButton = b;
    }

    this->setFocusProxy(firstButton);
    ui->leftBarrier->setBounceWidget(firstButton);
    ui->rightBarrier->setBounceWidget(lastButton);

    QWidget::setTabOrder(ui->leftBarrier, firstButton);
    QWidget::setTabOrder(lastButton, ui->rightBarrier);
}

void QuestionOverlay::setIcon(QMessageBox::Icon icon)
{
    QSize pixmapSize = SC_DPI_T(QSize(24, 24), QSize);

    d->icon = icon;
    switch (icon) {
        case QMessageBox::NoIcon:
            ui->iconLabel->setText("");
            break;
        case QMessageBox::Information:
            ui->iconLabel->setPixmap(QIcon(":/libentertaining/icons/dialog-information.svg").pixmap(pixmapSize));
            break;
        case QMessageBox::Warning:
            MusicEngine::playSoundEffect(MusicEngine::Warning);
            ui->iconLabel->setPixmap(QIcon(":/libentertaining/icons/dialog-warning.svg").pixmap(pixmapSize));
            break;
        case QMessageBox::Critical:
            MusicEngine::playSoundEffect(MusicEngine::Error);
            ui->iconLabel->setPixmap(QIcon(":/libentertaining/icons/dialog-error.svg").pixmap(pixmapSize));
            break;
        case QMessageBox::Question:
            ui->iconLabel->setPixmap(QIcon(":/libentertaining/icons/dialog-question.svg").pixmap(pixmapSize));
            break;

    }
}

void QuestionOverlay::reject()
{
    MusicEngine::playSoundEffect(MusicEngine::Backstep);
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([=] {
        emit rejected();
    });
}
