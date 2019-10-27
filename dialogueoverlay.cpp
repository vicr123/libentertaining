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
#include "dialogueoverlay.h"
#include "ui_dialogueoverlay.h"

#include <QKeyEvent>
#include <QEvent>
#include <QQueue>
#include <QPainter>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPushButton>
#include "gamepadevent.h"
#include <tvariantanimation.h>

#include "focusbarrier.h"

struct DialogueOverlayPrivate {
    QWidget* parent;

    bool haveOptions = false;

    QQueue<QString> pendingDialogues;
    QMap<QString, QString> lastOptions;

    QGraphicsOpacityEffect* textOpacity;
    QString remainingText;
    QTimer* textAnimationTimer;

    QWidget* optionSelectionWidget;
    QBoxLayout* optionSelectionWidgetLayout;
    QList<QPushButton*> optionsContents;
    QPushButton* selectedOption = nullptr;
};

DialogueOverlay::DialogueOverlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DialogueOverlay)
{
    ui->setupUi(this);

    d = new DialogueOverlayPrivate();
    d->parent = parent;

    this->setVisible(false);
    parent->installEventFilter(this);

    this->setFocusPolicy(Qt::StrongFocus);

    d->textOpacity = new QGraphicsOpacityEffect();
    d->textOpacity->setOpacity(0);
    ui->dialogFrame->setGraphicsEffect(d->textOpacity);

    d->textAnimationTimer = new QTimer(this);
    d->textAnimationTimer->setInterval(50);
    connect(d->textAnimationTimer, &QTimer::timeout, this, &DialogueOverlay::progressText);

    d->optionSelectionWidget = new QWidget(this);
    d->optionSelectionWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    d->optionSelectionWidgetLayout->setContentsMargins(0, 0, 0, 0);
    d->optionSelectionWidgetLayout->setSpacing(0);
    d->optionSelectionWidget->setLayout(d->optionSelectionWidgetLayout);
}

DialogueOverlay::~DialogueOverlay()
{
    delete ui;
    delete d;
}

void DialogueOverlay::setNewDialogue(QString dialogue, QMap<QString, QString> options)
{
    this->setMultiDialogue({dialogue}, options);
}

void DialogueOverlay::setMultiDialogue(QStringList dialogues, QMap<QString, QString> optionsForLastDialogue)
{
    d->pendingDialogues.append(dialogues);
    this->show();
    this->setFocus();

    d->lastOptions = optionsForLastDialogue;
    progressDialogue();
}

void DialogueOverlay::dismiss()
{
    showFrame(false);
}

void DialogueOverlay::progressDialogue()
{
    if (d->textAnimationTimer->isActive()) {
        skipTextAnimation();
        return;
    }

    if (d->pendingDialogues.isEmpty()) {
        //Tell everyone that we're finished
        emit progressDialogue("");
    } else {
        if (this->isVisible()) {
            this->show();
            this->showFrame(true);
        }

        ui->dialogText->setText("");

        QString currentDialogue = d->pendingDialogues.dequeue();
        d->remainingText = currentDialogue;
        d->textAnimationTimer->start();
    }
}

void DialogueOverlay::showFrame(bool show)
{
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(d->textOpacity->opacity());
    anim->setEndValue(show ? 1.0 : 0.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->textOpacity->setOpacity(value.toReal());
    });
    connect(anim, &tVariantAnimation::finished, this, [=] {
        anim->deleteLater();
        if (!show) this->hide();
    });
    anim->start();
}

void DialogueOverlay::progressText()
{
    QString text = ui->dialogText->text();
    text.append(d->remainingText.left(1));
    ui->dialogText->setText(text);
    d->remainingText = d->remainingText.mid(1);

    if (d->remainingText.isEmpty()) {
        //Perform the tasks to do after text animation is complete
        skipTextAnimation();
    }
}

void DialogueOverlay::skipTextAnimation()
{
    QString text = ui->dialogText->text();
    text.append(d->remainingText);
    ui->dialogText->setText(text);

    d->textAnimationTimer->stop();

    if (d->pendingDialogues.isEmpty() && d->lastOptions.count() > 0) {
        //Set and display the options
        int height = 0;

        FocusBarrier* topBarrier = new FocusBarrier();
        d->optionSelectionWidgetLayout->addWidget(topBarrier);
        d->optionsContents.append(topBarrier);

        QPushButton* firstButton = nullptr;
        QPushButton* bottomButton = nullptr;

        for (QString key : d->lastOptions.keys()) {
            QPushButton* button = new QPushButton();
            button->setText(d->lastOptions.value(key));
            button->installEventFilter(this);
            connect(button, &QPushButton::clicked, this, [=] {
                d->optionSelectionWidget->hide();

                //Clear the options contents
                for (QPushButton* button : d->optionsContents) {
                    d->optionSelectionWidgetLayout->removeWidget(button);
                    button->deleteLater();
                }
                d->optionsContents.clear();

                //Tell everyone we've got a button clicked
                emit progressDialogue(key);
            });
            d->optionSelectionWidgetLayout->addWidget(button);
            d->optionsContents.append(button);
            height += button->sizeHint().height();

            if (firstButton == nullptr) firstButton = button;
            bottomButton = button;
        }

        FocusBarrier* bottomBarrier = new FocusBarrier();
        d->optionSelectionWidgetLayout->addWidget(bottomBarrier);
        d->optionsContents.append(bottomBarrier);

        if (firstButton != nullptr) {
            topBarrier->setBounceWidget(firstButton);
            bottomBarrier->setBounceWidget(bottomButton);
        }

        firstButton->setFocus();
        this->setFocusProxy(firstButton);

        d->optionSelectionWidget->setFixedWidth(static_cast<int>(this->width() * 0.18));
        d->optionSelectionWidget->setFixedHeight(height);
        d->optionSelectionWidget->move(this->width() - d->optionSelectionWidget->width() - SC_DPI(20), this->height() - d->optionSelectionWidget->height() - SC_DPI(20));
        d->optionSelectionWidget->show();

        d->haveOptions = true;
    } else {
        d->haveOptions = false;
    }
}

void DialogueOverlay::keyReleaseEvent(QKeyEvent*event)
{
    if (event->key() == Qt::Key_Enter) {
        //Progress the dialogue if there are no options
        if (!d->haveOptions) {
            progressDialogue();
        }
    }
}

void DialogueOverlay::mouseReleaseEvent(QMouseEvent*event)
{
    //Progress the dialogue if there are no options
    if (!d->haveOptions) {
        progressDialogue();
    }
}

bool DialogueOverlay::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == d->parent && event->type() == QEvent::Resize) {
        this->resize(d->parent->width(), d->parent->height());
        ui->dialogFrame->setFixedWidth(static_cast<int>(d->parent->width() * 0.6));

        d->optionSelectionWidget->setFixedWidth(static_cast<int>(this->width() * 0.18));
        d->optionSelectionWidget->move(this->width() - d->optionSelectionWidget->width() - SC_DPI(20), this->height() - d->optionSelectionWidget->height() - SC_DPI(20));
    } else if (watched == ui->dialogFrame && event->type() == QEvent::Paint) {
        QPainter painter(ui->dialogFrame);
    } else if (d->optionsContents.contains(qobject_cast<QPushButton*>(watched))) {
        d->selectedOption = qobject_cast<QPushButton*>(watched);
    }
    return false;
}

bool DialogueOverlay::event(QEvent*event)
{
    if (event->type() == GamepadEvent::type()) {
        GamepadEvent* e = static_cast<GamepadEvent*>(event);
        if (e->isButtonEvent() && e->buttonPressed()) {
            if (e->button() == QGamepadManager::ButtonA) {
                //Advance the text or select the focused button
                if (d->haveOptions) {
                    d->selectedOption->click();
                } else {
                    progressDialogue();
                }
            }

            //Prevent propagation
            e->accept();
            return true;
        }
    }
    return QWidget::event(event);
}
