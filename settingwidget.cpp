#include "settingwidget.h"
#include "ui_settingwidget.h"

#include <QSettings>
#include <QKeyEvent>
#include <musicengine.h>
#include <tswitch.h>
#include <tsettings.h>
#include <QSlider>
#include "textinputoverlay.h"

struct SettingWidgetPrivate {
    tSettings settings;
    QVariant currentValue;

    SettingWidget::Type type;
    QString key;
    QWidget* stateWidget;
    QVariantMap metadata;
};

SettingWidget::SettingWidget(QWidget* parent, Type type, QString text, QString key, QVariantMap metadata) :
    QWidget(parent),
    ui(new Ui::SettingWidget) {
    ui->setupUi(this);

    d = new SettingWidgetPrivate();

    d->type = type;
    d->key = key;
    d->metadata = metadata;
    ui->textLabel->setText(text);
    d->currentValue = d->settings.value(key);

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
        case Range: {
            QSlider* s = new QSlider(this);
            s->setOrientation(Qt::Horizontal);
//            s->setFixedSize(SC_DPI(300), s->sizeHint().height());
            s->setMaximum(metadata.value("maximum", 100).toInt());
            s->setValue(d->currentValue.toInt());
            installStateWidget(s);

            connect(s, &QSlider::valueChanged, this, [ = ](int value) {
                d->currentValue = value;
                updateStateWidget();
                emit settingChanged(d->currentValue);
            });
            break;
        }
    }
}

SettingWidget::~SettingWidget() {
    delete d;
    delete ui;
}

void SettingWidget::activate() {
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
        case Range: {
            //Do Nothing
            break;
        }
    }
    updateStateWidget();
    emit settingChanged(d->currentValue);
}

void SettingWidget::updateSetting() {
    d->currentValue = d->settings.value(d->key);
    updateStateWidget();
}

void SettingWidget::keyPressEvent(QKeyEvent* event) {
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
        case Qt::Key_Down: {
            QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
            QApplication::sendEvent(this, &event);
            QKeyEvent event1(QKeyEvent::KeyRelease, Qt::Key_Tab, Qt::NoModifier);
            QApplication::sendEvent(this, &event1);
            break;
        }
        case Qt::Key_Right: {
            switch (d->type) {
                case Range: {
                    d->currentValue = d->currentValue.toInt() + d->metadata.value("step", 10).toInt();
                    updateStateWidget();
                    emit settingChanged(d->currentValue);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case Qt::Key_Left: {
            switch (d->type) {
                case Range: {
                    d->currentValue = d->currentValue.toInt() - d->metadata.value("step", 10).toInt();
                    updateStateWidget();
                    emit settingChanged(d->currentValue);
                    break;
                }
                default:
                    break;
            }
            break;
        }
    }
}

void SettingWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        event->accept();
    }
}

void SettingWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && this->geometry().contains(this->mapToParent(event->pos()))) {
        activate();
    }
}

void SettingWidget::focusInEvent(QFocusEvent* event) {
    emit hasFocus();
}

void SettingWidget::installStateWidget(QWidget* w) {
    w->setFocusPolicy(Qt::NoFocus);
    if (d->type != Range) w->setAttribute(Qt::WA_TransparentForMouseEvents);;
    ui->typeLayout->addWidget(w);
    d->stateWidget = w;
}

void SettingWidget::updateStateWidget() {
    switch (d->type) {
        case Boolean: {
            tSwitch* s = qobject_cast<tSwitch*>(d->stateWidget);
            s->setChecked(d->currentValue.toBool());
            d->settings.setValue(d->key, d->currentValue.toBool());
            break;
        }
        case Text: {
            QLabel* l = qobject_cast<QLabel*>(d->stateWidget);
            l->setText(d->currentValue.toString());
            d->settings.setValue(d->key, d->currentValue.toString());
            break;
        }
        case Range: {
            QSlider* s = qobject_cast<QSlider*>(d->stateWidget);
            s->setValue(d->currentValue.toInt());
            d->settings.setValue(d->key, d->currentValue.toInt());
        }
    }
}
