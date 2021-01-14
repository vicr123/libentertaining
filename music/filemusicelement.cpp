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
#include "musicengine.h"

struct FileMusicElementPrivate {
    QAudioFormat format;
    QAudioOutput* output;
    QIODevice* sink = nullptr;

    QByteArray backgroundStart, backgroundLoop;
    quint64 audioPointer = 0;

    QQueue<QUrl> backgroundStartUrls;
    QQueue<QUrl> backgroundLoopUrls;
    QString backgroundStartResource;
    QString backgroundLoopResource;
    bool haveStart = false;
    bool isPlaying = false;
    bool isSuspended = false;
};

FileMusicElement::FileMusicElement(QString startResource, QString loopResource, QObject* parent) : AbstractMusicElement(parent) {
    init();
    d->sink = d->output->start();

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

FileMusicElement::FileMusicElement(QUrl startUrl, QUrl loopUrl, QObject* parent) : AbstractMusicElement(parent) {
    init();
    d->sink = d->output->start();

    if (startUrl.isValid()) {
        d->backgroundStartUrls.append(startUrl);
        tryNextBackgroundStart();
        d->haveStart = true;
    } else {
        d->haveStart = false;
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

    d->output = new QAudioOutput(d->format, this);
    d->output->setBufferSize(d->format.sampleRate() * d->format.channelCount() / 4);
    d->output->setNotifyInterval(100);
    connect(d->output, &QAudioOutput::notify, this, [ = ] {
        fillAudioBuffer();
    });
    connect(d->output, &QAudioOutput::stateChanged, this, [ = ](QAudio::State state) {
        if (state == QAudio::IdleState) {
            fillAudioBuffer();
        }
    });
}

void FileMusicElement::fillAudioBuffer() {
    //Make sure the background sink is open
    if (!d->sink) return;

    quint64 startData = d->backgroundStart.length();
    quint64 totalData = d->backgroundLoop.length() + startData;
    if (totalData == 0) {
        if (!d->isSuspended) d->output->suspend();
        d->isSuspended = true;
        return;
    }

    quint64 free = d->output->bytesFree();
    if (d->audioPointer < startData) {
        //Continue to read in the start data
        QByteArray data = d->backgroundStart.mid(d->audioPointer, free);
        free -= data.length();
        d->audioPointer += data.length();
        d->sink->write(data);
    }

    while (free != 0 && d->backgroundLoop.length() != 0) {
        //Continue to read in the loop
        QByteArray data = d->backgroundLoop.mid(d->audioPointer - startData, free);
        free -= data.length();
        d->audioPointer += data.length();
        d->sink->write(data);
        if (d->audioPointer >= totalData) d->audioPointer = startData;
    }

    if (d->isPlaying && d->isSuspended) {
        d->output->resume();
        d->isSuspended = false;
    }
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
            if (d->backgroundLoop.length() > d->output->bufferSize() && (!d->haveStart || d->backgroundStart.length() > d->output->bufferSize())) fillAudioBuffer();
        });
        connect(initialDecoder, &QAudioDecoder::finished, [ = ] {
            initialDecoder->deleteLater();
        });
        connect(initialDecoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), this, [ = ](QAudioDecoder::Error error) {
            tryNextBackgroundStart();
            initialDecoder->deleteLater();
        });
        initialDecoder->start();

        if (d->isPlaying) play();
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
            if (d->backgroundLoop.length() > d->output->bufferSize() && (!d->haveStart || d->backgroundStart.length() > d->output->bufferSize())) fillAudioBuffer();
        });
        connect(loopDecoder, &QAudioDecoder::finished, [ = ] {
            loopDecoder->deleteLater();
        });
        connect(loopDecoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), this, [ = ](QAudioDecoder::Error error) {
            tryNextBackgroundLoop();
            loopDecoder->deleteLater();
        });
        loopDecoder->start();

        if (d->isPlaying) play();
    }
}


void FileMusicElement::play() {
    if (d->isSuspended) d->output->resume();
    d->isSuspended = false;
    d->isPlaying = true;
}

void FileMusicElement::pause() {
    if (!d->isSuspended) d->output->suspend();
    d->isSuspended = true;
    d->isPlaying = false;
}

void FileMusicElement::setVolume(qreal volume) {
    d->output->setVolume(volume);
}
