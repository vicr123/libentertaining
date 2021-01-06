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
#include "musicengine.h"

#include <QMediaPlayer>
#include <QSoundEffect>
#include <QSound>
#include <QDir>
#include <QQueue>
#include <QAudioOutput>
#include <QAudioDecoder>
#include <QPointer>
#include <tapplication.h>

struct MusicEnginePrivate {
    MusicEngine* instance = nullptr;

//    QMediaPlayer* backgroundMusic;
    QAudioOutput* backgroundOutput = nullptr;
    QPointer<QIODevice> backgroundSink = nullptr;
    bool playingBackgroudMusic = false;
    bool isBackgroundSuspended = false;

    QAudioFormat format;

    QByteArray backgroundStart, backgroundLoop;
    quint64 audioPointer = 0;

    QQueue<QUrl> backgroundStartUrls;
    QQueue<QUrl> backgroundLoopUrls;
    QString backgroundStartResource;
    QString backgroundLoopResource;
    bool haveStart = false;

    QList<QSoundEffect*> activeEffects;
    bool muteEffects = false;

    const QMap<MusicEngine::KnownSoundEffect, QUrl> soundEffectUrls = {
        {MusicEngine::FocusChanged, QUrl("qrc:/libentertaining/audio/focusmove.wav")},
        {MusicEngine::FocusChangedFailed, QUrl("qrc:/libentertaining/audio/back.wav")},
        {MusicEngine::Pause, QUrl("qrc:/libentertaining/audio/pause.wav")},
        {MusicEngine::Selection, QUrl("qrc:/libentertaining/audio/select.wav")},
        {MusicEngine::Backstep, QUrl("qrc:/libentertaining/audio/back.wav")},
        {MusicEngine::Warning, QUrl("qrc:/libentertaining/audio/warning.wav")},
        {MusicEngine::Error, QUrl("qrc:/libentertaining/audio/error.wav")}
    };

};

MusicEnginePrivate* MusicEngine::d = new MusicEnginePrivate();

void MusicEngine::setBackgroundMusic(QUrl path) {
    ensureInstance();

    d->backgroundLoopUrls.clear();
//    d->backgroundMusic->setMedia(QMediaContent(path));
    setBackgroundMusic(QUrl(), path);
    if (d->playingBackgroudMusic) playBackgroundMusic();
}

void MusicEngine::setBackgroundMusic(QString audioResource) {
    setBackgroundMusic("", audioResource);
}

void MusicEngine::setBackgroundMusic(QUrl initialPath, QUrl loopingPath) {
    ensureInstance();
    d->backgroundStartResource.clear();
    d->backgroundLoopResource.clear();

    d->audioPointer = 0;
    d->backgroundOutput->stop();
    d->backgroundSink = d->backgroundOutput->start();

    d->backgroundStartUrls.clear();
    d->backgroundLoopUrls.clear();

    if (initialPath.isValid()) {
        d->backgroundStartUrls.append(initialPath);
        tryNextBackgroundStart();
        d->haveStart = true;
    } else {
        d->haveStart = false;
    }

    d->backgroundLoopUrls.append(loopingPath);
    tryNextBackgroundLoop();
}

void MusicEngine::setBackgroundMusic(QString initialResource, QString loopingResource) {
    ensureInstance();
    if (d->backgroundStartResource == initialResource && d->backgroundLoopResource == loopingResource) return;
    d->backgroundStartResource = initialResource;
    d->backgroundLoopResource = loopingResource;

    d->audioPointer = 0;
    d->backgroundOutput->stop();
    d->backgroundSink = d->backgroundOutput->start();

    d->backgroundStartUrls.clear();
    if (initialResource.isEmpty()) {
        d->haveStart = false;
    } else {
        d->haveStart = true;
        for (const QUrl& url : resolveAudioResource(initialResource)) {
            d->backgroundStartUrls.enqueue(url);
        }
    }

    d->backgroundLoopUrls.clear();
    for (const QUrl& url : resolveAudioResource(loopingResource)) {
        d->backgroundLoopUrls.enqueue(url);
    }

    tryNextBackgroundStart();
    tryNextBackgroundLoop();
}

void MusicEngine::playBackgroundMusic() {
    ensureInstance();

//    d->backgroundMusic->play();
    if (d->isBackgroundSuspended) d->backgroundOutput->resume();
    d->isBackgroundSuspended = false;
    d->playingBackgroudMusic = true;
}

void MusicEngine::pauseBackgroundMusic() {
    ensureInstance();

//    d->backgroundMusic->pause();
    if (!d->isBackgroundSuspended) d->backgroundOutput->suspend();
    d->isBackgroundSuspended = true;
    d->playingBackgroudMusic = false;
}

void MusicEngine::setUserBackgroundVolume(qreal volume) {
    ensureInstance();

    d->backgroundOutput->setVolume(volume);
}

qreal MusicEngine::userBackgroundVolume() {
    ensureInstance();

    return d->backgroundOutput->volume();
}

void MusicEngine::playSoundEffect(MusicEngine::KnownSoundEffect effect) {
    MusicEngine::playSoundEffect(d->soundEffectUrls.value(effect));
}

void MusicEngine::playSoundEffect(QUrl path) {
    if (d->muteEffects) return;

    QSoundEffect* effect = new QSoundEffect();
    effect->setSource(path);
    effect->play();
    connect(effect, &QSoundEffect::playingChanged, effect, [ = ] {
        if (!effect->isPlaying()) {
            d->activeEffects.removeAll(effect);
            effect->deleteLater();
        }
    });
    d->activeEffects.append(effect);
}

void MusicEngine::playSoundEffect(QString audioResource) {
    QList<QUrl> urls = resolveAudioResource(audioResource);
    if (urls.count() > 0) playSoundEffect(urls.first());
}

void MusicEngine::setMuteEffects(bool mute) {
    ensureInstance();

    d->muteEffects = mute;
}

bool MusicEngine::isEffectsMuted() {
    ensureInstance();

    return d->muteEffects;
}

MusicEngine::MusicEngine(QObject* parent) : QObject(parent) {
//    d->backgroundMusic = new QMediaPlayer();
//    connect(d->backgroundMusic, &QMediaPlayer::mediaStatusChanged, this, [ = ](QMediaPlayer::MediaStatus status) {
//        if (status == QMediaPlayer::EndOfMedia) {
//            //Loop the media
//            d->backgroundMusic->setPosition(0);
//            d->backgroundMusic->play();
//        }
//    });
//    connect(d->backgroundMusic, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, [ = ](QMediaPlayer::Error error) {
//        qDebug() << "Qt Multimedia Error" << error;
//        tryNextBackgroundTrack();
//    });

    d->format.setSampleRate(44100);
    d->format.setChannelCount(2);
    d->format.setSampleSize(8);
    d->format.setCodec("audio/pcm");
    d->format.setByteOrder(QAudioFormat::LittleEndian);
    d->format.setSampleType(QAudioFormat::UnSignedInt);

    d->backgroundOutput = new QAudioOutput(d->format);
    d->backgroundOutput->setBufferSize(d->format.sampleRate() * d->format.channelCount() / 4);
    d->backgroundOutput->setNotifyInterval(100);
    connect(d->backgroundOutput, &QAudioOutput::notify, this, [ = ] {
        fillAudioBuffer();
    });
    connect(d->backgroundOutput, &QAudioOutput::stateChanged, this, [ = ](QAudio::State state) {
        if (state == QAudio::IdleState) {
            fillAudioBuffer();
        }
    });
}

void MusicEngine::ensureInstance() {
    if (d->instance == nullptr) d->instance = new MusicEngine();
}

void MusicEngine::tryNextBackgroundStart() {
    if (d->backgroundStartUrls.count() > 0) {
        QUrl fileUrl = d->backgroundStartUrls.dequeue();
        qDebug() << "Background music: Trying" << fileUrl;

        d->backgroundStart.clear();
        QAudioDecoder* initialDecoder = new QAudioDecoder();
        initialDecoder->setSourceFilename(fileUrl.toLocalFile());
        initialDecoder->setAudioFormat(d->format);
        connect(initialDecoder, &QAudioDecoder::bufferReady, [ = ] {
            QAudioBuffer buf = initialDecoder->read();
            d->backgroundStart.append(buf.constData<char>(), buf.byteCount());
            if (d->backgroundLoop.length() > d->backgroundOutput->bufferSize() && (!d->haveStart || d->backgroundStart.length() > d->backgroundOutput->bufferSize())) d->instance->fillAudioBuffer();
        });
        connect(initialDecoder, &QAudioDecoder::finished, [ = ] {
            initialDecoder->deleteLater();
        });
        connect(initialDecoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), [ = ](QAudioDecoder::Error error) {
            tryNextBackgroundStart();
            initialDecoder->deleteLater();
        });
        initialDecoder->start();

        if (d->playingBackgroudMusic) playBackgroundMusic();
    }
}

void MusicEngine::tryNextBackgroundLoop() {
    if (d->backgroundLoopUrls.count() > 0) {
        QUrl fileUrl = d->backgroundLoopUrls.dequeue();
        qDebug() << "Background music: Trying" << fileUrl;

        d->backgroundLoop.clear();
        QAudioDecoder* loopDecoder = new QAudioDecoder();
        loopDecoder->setSourceFilename(fileUrl.toLocalFile());
        loopDecoder->setAudioFormat(d->format);
        connect(loopDecoder, &QAudioDecoder::bufferReady, [ = ] {
            QAudioBuffer buf = loopDecoder->read();
            d->backgroundLoop.append(buf.constData<char>(), buf.byteCount());
            if (d->backgroundLoop.length() > d->backgroundOutput->bufferSize() && (!d->haveStart || d->backgroundStart.length() > d->backgroundOutput->bufferSize())) d->instance->fillAudioBuffer();
        });
        connect(loopDecoder, &QAudioDecoder::finished, [ = ] {
            loopDecoder->deleteLater();
        });
        connect(loopDecoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), [ = ](QAudioDecoder::Error error) {
            tryNextBackgroundLoop();
            loopDecoder->deleteLater();
        });
        loopDecoder->start();

        if (d->playingBackgroudMusic) playBackgroundMusic();
    }
}

QList<QUrl> MusicEngine::resolveAudioResource(QString audioResource) {
    QList<QUrl> resources;
    QString mediaDir;

#if defined(Q_OS_MAC)
    mediaDir = tApplication::macOSBundlePath() + "/Contents/audio";
#elif defined(Q_OS_LINUX)
    mediaDir = tApplication::shareDir() + "/audio";
#elif defined(Q_OS_WIN)
    mediaDir = tApplication::applicationDirPath() + "\\audio";
#endif

    QDir dir(mediaDir);
    for (const QFileInfo& fileInfo : dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        if (fileInfo.baseName() == audioResource) {
            resources.append(QUrl::fromLocalFile(fileInfo.filePath()));
        }
    }
    return resources;
}

void MusicEngine::fillAudioBuffer() {
    //Make sure the background sink is open
    if (!d->backgroundSink) return;

    quint64 startData = d->backgroundStart.length();
    quint64 totalData = d->backgroundLoop.length() + startData;
    if (totalData == 0) {
        if (!d->isBackgroundSuspended) d->backgroundOutput->suspend();
        d->isBackgroundSuspended = true;
        return;
    }

    quint64 free = d->backgroundOutput->bytesFree();
    if (d->audioPointer < startData) {
        //Continue to read in the start data
        QByteArray data = d->backgroundStart.mid(d->audioPointer, free);
        free -= data.length();
        d->audioPointer += data.length();
        d->backgroundSink->write(data);
    }

    while (free != 0 && d->backgroundLoop.length() != 0) {
        //Continue to read in the loop
        QByteArray data = d->backgroundLoop.mid(d->audioPointer - startData, free);
        free -= data.length();
        d->audioPointer += data.length();
        d->backgroundSink->write(data);
        if (d->audioPointer >= totalData) d->audioPointer = startData;
    }

    if (d->playingBackgroudMusic && d->isBackgroundSuspended) {
        d->backgroundOutput->resume();
        d->isBackgroundSuspended = false;
    }
}
