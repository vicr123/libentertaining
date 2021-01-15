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
#ifndef FILEMUSICELEMENT_H
#define FILEMUSICELEMENT_H

#include "abstractmusicelement.h"

struct FileMusicElementPrivate;
class FileMusicElement : public AbstractMusicElement {
        Q_OBJECT
    public:
        explicit FileMusicElement(QString trackName, QString startResource, QString loopResource, QObject* parent = nullptr);
        explicit FileMusicElement(QString trackName, QList<QUrl> startUrl, QList<QUrl> loopUrl, QObject* parent = nullptr);
        ~FileMusicElement();

    signals:

    private:
        FileMusicElementPrivate* d;

        void init();
        void tryNextBackgroundStart();
        void tryNextBackgroundLoop();

        // AbstractMusicElement interface
    public:
        QByteArray data(quint64 offset, quint64 length);
        bool blocking(quint64 bufferSize);
        void setStreamVolume(QString trackName, qreal volume);
};

#endif // FILEMUSICELEMENT_H
