/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#ifndef ABSTRACTMUSICELEMENT_H
#define ABSTRACTMUSICELEMENT_H

#include <QObject>

class AbstractMusicElement : public QObject {
        Q_OBJECT
    public:
        explicit AbstractMusicElement(QString trackName, QObject* parent = nullptr);

        virtual QByteArray data(quint64 offset, quint64 length) = 0;
        virtual bool blocking(quint64 bufferSize) = 0;
        virtual void setStreamVolume(QString trackName, qreal volume) = 0;

    signals:
        void attemptBufferFill();

};

#endif // ABSTRACTMUSICELEMENT_H
