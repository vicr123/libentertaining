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
#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>

namespace Ui {
    class ReportWidget;
}

struct ReportWidgetPrivate;
class ReportWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ReportWidget(QWidget *parent = nullptr);
        ~ReportWidget();

        void beginScreenReport(QWidget* widget, QVariantMap details);

    private slots:
        void on_explainPageContinue_clicked();

        void on_backButton_clicked();

        void on_backButton_2_clicked();

    signals:
        void done();

    private:
        Ui::ReportWidget *ui;
        ReportWidgetPrivate* d;

        void show();
        void close();
        bool eventFilter(QObject* watched, QEvent* event);
};

class ReportControllerEventFilter : public QObject {
        Q_OBJECT

    public:
        ReportControllerEventFilter();

    private:
        bool eventFilter(QObject* watched, QEvent* event);

        bool startPressed = false;
        bool selPressed = false;
};

#endif // REPORTWIDGET_H
