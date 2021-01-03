#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

#include <QWidget>
#include <QVariantMap>
#include "libentertaining_global.h"

namespace Ui {
    class SettingWidget;
}

struct SettingWidgetPrivate;
class LIBENTERTAINING_EXPORT SettingWidget : public QWidget {
        Q_OBJECT

    public:
        enum Type {
            Boolean,
            Text,
            Range
        };

        explicit SettingWidget(QWidget* parent, Type type, QString text, QString key, QVariantMap metadata = QVariantMap());
        ~SettingWidget();

    public slots:
        void activate();
        void updateSetting();

    signals:
        void settingChanged(QVariant newValue);
        void hasFocus();

    private:
        Ui::SettingWidget* ui;
        SettingWidgetPrivate* d;

        void keyPressEvent(QKeyEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void focusInEvent(QFocusEvent* event);
        void installStateWidget(QWidget* w);

        void updateStateWidget();
};

#endif // SETTINGWIDGET_H
