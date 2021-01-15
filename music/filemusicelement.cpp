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
#include "filemusicelement.h"

#include <QQueue>
#include <QUrl>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QDebug>
#include <QAudioDecoder>
#include <QtConcurrent>
#include "musicengine.h"

struct FileMusicElementPrivate {
    QAudioFormat format;

    QByteArray backgroundStart, backgroundLoop;

    QQueue<QUrl> backgroundStartUrls;
    QQueue<QUrl> backgroundLoopUrls;
    QString backgroundStartResource;
    QString backgroundLoopResource;
    bool haveStart = false;

    QString trackName;
    qreal volume = 1;
};

FileMusicElement::FileMusicElement(QString trackName, QString startResource, QString loopResource, QObject* parent) : AbstractMusicElement(trackName, parent) {
    init();
    d->trackName = trackName;

    if (startResource.isEmpty()) {
        d->haveStart = false;
    } else {
        d->haveStart = true;
        for (const QUrl& url : MusicEngine::resolveAudioResource(startResource)) {
            d->backgroundStartUrls.enqueue(url);
        }
    }

    d->backgroundLoopUrls.clear();
    for (const QUrl& url : MusicEngine::resolveAudioResource(loopResource)) {
        d->backgroundLoopUrls.enqueue(url);
    }

    tryNextBackgroundStart();
    tryNextBackgroundLoop();
}

FileMusicElement::FileMusicElement(QString trackName, QList<QUrl> startUrl, QList<QUrl> loopUrl, QObject* parent) : AbstractMusicElement(trackName, parent) {
    init();
    d->trackName = trackName;

    if (startUrl.isEmpty()) {
        d->haveStart = false;
    } else {
        d->backgroundStartUrls.append(startUrl);
        tryNextBackgroundStart();
        d->haveStart = true;
    }

    d->backgroundLoopUrls.append(loopUrl);
    tryNextBackgroundLoop();
}

FileMusicElement::~FileMusicElement() {
    delete d;
}

void FileMusicElement::init() {
    d = new FileMusicElementPrivate();

    d->format.setSampleRate(44100);
    d->format.setChannelCount(2);
    d->format.setSampleSize(8);
    d->format.setCodec("audio/pcm");
    d->format.setByteOrder(QAudioFormat::LittleEndian);
    d->format.setSampleType(QAudioFormat::UnSignedInt);
}

void FileMusicElement::tryNextBackgroundStart() {
    if (d->backgroundStartUrls.count() > 0) {
        QUrl fileUrl = d->backgroundStartUrls.dequeue();
        qDebug() << "Background music: Trying" << fileUrl;

        d->backgroundStart.clear();
        QAudioDecoder* initialDecoder = new QAudioDecoder();
        initialDecoder->setSourceFilename(fileUrl.toLocalFile());
        initialDecoder->setAudioFormat(d->format);
        connect(initialDecoder, &QAudioDecoder::bufferReady, this, [ = ] {
            QAudioBuffer buf = initialDecoder->read();
            d->backgroundStart.append(buf.constData<char>(), buf.byteCount());
        });
        connect(initialDecoder, &QAudioDecoder::finished, [ = ] {
            initialDecoder->deleteLater();
            emit attemptBufferFill();
        });
        connect(initialDecoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), this, [ = ](QAudioDecoder::Error error) {
            tryNextBackgroundStart();
            initialDecoder->deleteLater();
        });
        initialDecoder->start();
    }
}

void FileMusicElement::tryNextBackgroundLoop() {
    if (d->backgroundLoopUrls.count() > 0) {
        QUrl fileUrl = d->backgroundLoopUrls.dequeue();
        qDebug() << "Background music: Trying" << fileUrl;

        d->backgroundLoop.clear();
        QAudioDecoder* loopDecoder = new QAudioDecoder();
        loopDecoder->setSourceFilename(fileUrl.toLocalFile());
        loopDecoder->setAudioFormat(d->format);
        connect(loopDecoder, &QAudioDecoder::bufferReady, this, [ = ] {
            QAudioBuffer buf = loopDecoder->read();
            d->backgroundLoop.append(buf.constData<char>(), buf.byteCount());
        });
        connect(loopDecoder, &QAudioDecoder::finished, [ = ] {
            loopDecoder->deleteLater();
            emit attemptBufferFill();
        });
        connect(loopDecoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), this, [ = ](QAudioDecoder::Error error) {
            tryNextBackgroundLoop();
            loopDecoder->deleteLater();
        });
        loopDecoder->start();
    }
}

QByteArray FileMusicElement::data(quint64 offset, quint64 length) {
    quint64 startData = d->backgroundStart.length();
    quint64 loopData = d->backgroundLoop.length();
    quint64 totalData = loopData + startData;
    if (totalData == 0) {
        return QByteArray(length, 0);
    }

    quint64 audioPointer = offset;
    QByteArray audioData;

    quint64 free = length;
    if (audioPointer < startData) {
        //Continue to read in the start data
        QByteArray data = d->backgroundStart.mid(audioPointer, free);
        free -= data.length();
        audioPointer += data.length();
        audioData.append(data);
    }

    while (free != 0 && d->backgroundLoop.length() != 0) {
        //Continue to read in the loop
        QByteArray data = d->backgroundLoop.mid((audioPointer - startData) % loopData, free);
        free -= data.length();
        audioPointer += data.length();
        audioData.append(data);
        if (audioPointer >= totalData) audioPointer = startData;
    }

    //Attenuate the data depending on the volume
    QtConcurrent::blockingMap(audioData, [ = ](char& data) {
        uchar* udata = reinterpret_cast<uchar*>(&data);
        *udata *= d->volume;
    });

    return audioData;
}

bool FileMusicElement::blocking(quint64 bufferSize) {
    if (static_cast<quint64>(d->backgroundLoop.length()) > bufferSize && (!d->haveStart || static_cast<quint64>(d->backgroundStart.length()) > bufferSize)) {
        return d->backgroundStart.length() + d->backgroundLoop.length() == 0;
    } else {
        return true;
    }
}

void FileMusicElement::setStreamVolume(QString trackName, qreal volume) {
    if (d->trackName == trackName) d->volume = volume;
}
