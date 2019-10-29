#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

#include <QWidget>
#include <QVariantMap>

namespace Ui {
    class SettingWidget;
}

struct SettingWidgetPrivate;
class SettingWidget : public QWidget
{
        Q_OBJECT

    public:
        enum Type {
            Boolean,
            Text
        };

        explicit SettingWidget(QWidget* parent, Type type, QString text, QString key, QVariant defaultValue, QVariantMap metadata = QVariantMap());
        ~SettingWidget();

    public slots:
        void activate();
        void updateSetting();

    signals:
        void settingChanged(QVariant newValue);
        void hasFocus();

    private:
        Ui::SettingWidget *ui;
        SettingWidgetPrivate* d;

        void keyPressEvent(QKeyEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void focusInEvent(QFocusEvent* event);
        void installStateWidget(QWidget* w);

        void updateStateWidget();
};

#endif // SETTINGWIDGET_H
