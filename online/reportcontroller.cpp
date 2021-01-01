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
#include "reportcontroller.h"

#include <QApplication>
#include <QPointer>
#include <QWidget>
#include "private/reportwidget.h"

struct ReportControllerPrivate {
    static QList<QWidget*> reportingEnabledFor;
    static QMap<QWidget*, QVariantMap> reportingDetails;
    static ReportControllerEventFilter* filter;
    static QPointer<QWidget> focusRestore;
};
ReportControllerEventFilter* ReportControllerPrivate::filter = nullptr;
QList<QWidget*> ReportControllerPrivate::reportingEnabledFor = QList<QWidget*>();
QMap<QWidget*, QVariantMap> ReportControllerPrivate::reportingDetails = QMap<QWidget*, QVariantMap>();
QPointer<QWidget> ReportControllerPrivate::focusRestore = QPointer<QWidget>();

void ReportController::setAutomaticReportingEnabled(QWidget* parent, bool reportingEnabled) {
    if (ReportControllerPrivate::filter == nullptr) ReportControllerPrivate::filter = new ReportControllerEventFilter();

    QWidget* w = parent->window();
    if (ReportControllerPrivate::reportingEnabledFor.contains(w) || !reportingEnabled) {
        ReportControllerPrivate::reportingEnabledFor.removeAll(w);
        w->removeEventFilter(ReportControllerPrivate::filter);
    } else {
        ReportControllerPrivate::reportingEnabledFor.append(w);
        w->installEventFilter(ReportControllerPrivate::filter);

        connect(w, &QWidget::destroyed, [ = ] {
            ReportControllerPrivate::reportingEnabledFor.removeAll(w);
        });
    }
}

void ReportController::setReportDetails(QWidget* parent, QVariantMap details) {
    ReportControllerPrivate::reportingDetails.insert(parent->window(), details);
}

void ReportController::beginScreenReport(QWidget* parent) {
    if (parent == nullptr) return;
    ReportControllerPrivate::focusRestore = QApplication::focusWidget();
    QWidget* w = parent->window();

    w->removeEventFilter(ReportControllerPrivate::filter);
    ReportWidget* reporter = new ReportWidget(w);
    reporter->beginScreenReport(parent, ReportControllerPrivate::reportingDetails.value(w, QVariantMap()));
    connect(reporter, &ReportWidget::done, [ = ] {
        if (ReportControllerPrivate::reportingEnabledFor.contains(w)) {
            w->installEventFilter(ReportControllerPrivate::filter);
        }

        if (ReportControllerPrivate::focusRestore) {
            ReportControllerPrivate::focusRestore->setFocus();
            ReportControllerPrivate::focusRestore.clear();
        }
    });
}

ReportController::ReportController(QObject* parent) : QObject(parent) {

}
