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
#ifndef REPORTCONTROLLER_H
#define REPORTCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include "libentertaining_global.h"

struct ReportControllerPrivate;
class LIBENTERTAINING_EXPORT ReportController : public QObject
{
        Q_OBJECT
    public:
        static void setAutomaticReportingEnabled(QWidget* parent, bool reportingEnabled);
        static void setReportDetails(QWidget* parent, QVariantMap details);
        static void beginScreenReport(QWidget* parent);

    signals:

    public slots:

    private:
        explicit ReportController(QObject *parent = nullptr);

        ReportControllerPrivate* d;
};

#endif // REPORTCONTROLLER_H
