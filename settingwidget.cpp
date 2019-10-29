#include "settingwidget.h"
#include "ui_settingwidget.h"

#include <QSettings>
#include <QKeyEvent>
#include <musicengine.h>
#include <tswitch.h>
#include "textinputoverlay.h"

struct SettingWidgetPrivate {
    QSettings settings;
    QVariant currentValue;

    SettingWidget::Type type;
    QString key;
    QWidget* stateWidget;
    QVariantMap metadata;
    QVariant defaultValue;
};

SettingWidget::SettingWidget(QWidget *parent, Type type, QString text, QString key, QVariant defaultValue, QVariantMap metadata) :
    QWidget(parent),
    ui(new Ui::SettingWidget)
{
    ui->setupUi(this);

    d = new SettingWidgetPrivate();

    d->type = type;
    d->key = key;
    d->metadata = metadata;
    d->defaultValue = defaultValue;
    ui->textLabel->setText(text);
    d->currentValue = d->settings.value(key, defaultValue);

    switch (type) {
        case Boolean: {
            tSwitch* s = new tSwitch(this);
            s->setChecked(d->currentValue.toBool());
            installStateWidget(s);
            break;
        }
        case Text: {
            QLabel* l = new QLabel(this);
            l->setText(d->currentValue.toString());
            installStateWidget(l);
            break;
        }
    }
}

SettingWidget::~SettingWidget()
{
    delete d;
    delete ui;
}

void SettingWidget::activate()
{
    MusicEngine::playSoundEffect(MusicEngine::Selection);
    switch (d->type) {
        case Boolean: {
            d->currentValue = !d->currentValue.toBool();
            break;
        }
        case Text: {
            bool canceled;
            QString set = TextInputOverlay::getText(d->metadata.value("popoverOver", QVariant::fromValue(this->parentWidget())).value<QWidget*>(), ui->textLabel->text(), &canceled, d->currentValue.toString(), d->metadata.value("noEcho", false).toBool() ? QLineEdit::Password : QLineEdit::Normal);
            if (!canceled) {
                d->currentValue = set;
            }
        }
    }
    updateStateWidget();
    emit settingChanged(d->currentValue);
}

void SettingWidget::updateSetting()
{
    d->currentValue = d->settings.value(d->key, d->defaultValue);
    updateStateWidget();
}

void SettingWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Space:
            this->activate();
            break;
        case Qt::Key_Up: {
            QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
            QApplication::sendEvent(this, &event);
            QKeyEvent event1(QKeyEvent::KeyRelease, Qt::Key_Backtab, Qt::NoModifier);
            QApplication::sendEvent(this, &event1);
            break;
        }
        case Qt::Key_Down:{
            QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
            QApplication::sendEvent(this, &event);
            QKeyEvent event1(QKeyEvent::KeyRelease, Qt::Key_Tab, Qt::NoModifier);
            QApplication::sendEvent(this, &event1);
        }
    }
}

void SettingWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        event->accept();
    }
}

void SettingWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && this->geometry().contains(this->mapToParent(event->pos()))) {
        activate();
    }
}

void SettingWidget::focusInEvent(QFocusEvent* event)
{
    emit hasFocus();
}

void SettingWidget::installStateWidget(QWidget* w)
{
    w->setFocusPolicy(Qt::NoFocus);
    w->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->typeLayout->addWidget(w);
    d->stateWidget = w;
}

void SettingWidget::updateStateWidget()
{
    switch (d->type) {
        case Boolean: {
            tSwitch* s = qobject_cast<tSwitch*>(d->stateWidget);
            s->setChecked(d->currentValue.toBool());
            d->settings.setValue(d->key, d->currentValue);
            break;
        }
        case Text: {
            QLabel* l = qobject_cast<QLabel*>(d->stateWidget);
            l->setText(d->currentValue.toString());
            d->settings.setValue(d->key, d->currentValue.toString());
        }
    }
}
