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
#include <tapplication.h>

struct MusicEnginePrivate {
    MusicEngine* instance = nullptr;

    QMediaPlayer* backgroundMusic;
    bool playingBackgroudMusic = false;

    QQueue<QUrl> backgroundMusicUrls;

    QList<QSoundEffect*> activeEffects;
    bool muteEffects = false;

    const QMap<MusicEngine::KnownSoundEffect, QUrl> soundEffectUrls = {
        {MusicEngine::FocusChanged, QUrl("qrc:/libentertaining/audio/focusmove.wav")},
        {MusicEngine::FocusChangedFailed, QUrl("qrc:/libentertaining/audio/back.wav")},
        {MusicEngine::Pause, QUrl("qrc:/libentertaining/audio/pause.wav")},
        {MusicEngine::Selection, QUrl("qrc:/libentertaining/audio/select.wav")},
        {MusicEngine::Backstep, QUrl("qrc:/libentertaining/audio/back.wav")}
    };

};

MusicEnginePrivate* MusicEngine::d = new MusicEnginePrivate();

void MusicEngine::setBackgroundMusic(QUrl path)
{
    ensureInstance();

    d->backgroundMusicUrls.clear();
    d->backgroundMusic->setMedia(QMediaContent(path));
    if (d->playingBackgroudMusic) playBackgroundMusic();
}

void MusicEngine::setBackgroundMusic(QString audioResource)
{
    ensureInstance();

    d->backgroundMusicUrls.clear();

    QString mediaDir;

#if defined(Q_OS_MAC)
    mediaDir = tApplication::macOSBundlePath() + "/Contents/audio";
#elif defined(Q_OS_LINUX)
    mediaDir = tApplication::shareDir() + "/audio";
#elif defined(Q_OS_WIN)
    mediaDir = tApplication::applicationDirPath() + "\\audio";
#endif

    QDir dir(mediaDir);
    for (QFileInfo fileInfo : dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        if (fileInfo.baseName() == audioResource) {
            d->backgroundMusicUrls.enqueue(QUrl::fromLocalFile(fileInfo.filePath()));
        }
    }

    tryNextBackgroundTrack();
}

void MusicEngine::playBackgroundMusic()
{
    ensureInstance();

    d->backgroundMusic->play();
    d->playingBackgroudMusic = true;
}

void MusicEngine::pauseBackgroundMusic()
{
    ensureInstance();

    d->backgroundMusic->pause();
    d->playingBackgroudMusic = false;
}

void MusicEngine::setMuteMusic(bool mute)
{
    ensureInstance();

    d->backgroundMusic->setVolume(mute ? 0 : 100);
}

bool MusicEngine::isMusicMuted()
{
    ensureInstance();

    return d->backgroundMusic->volume() == 0 ? true : false;
}

void MusicEngine::playSoundEffect(MusicEngine::KnownSoundEffect effect)
{
    MusicEngine::playSoundEffect(d->soundEffectUrls.value(effect));
}

void MusicEngine::playSoundEffect(QUrl path)
{
    if (d->muteEffects) return;

    QSoundEffect* effect = new QSoundEffect();
    effect->setSource(path);
    effect->play();
    connect(effect, &QSoundEffect::playingChanged, effect, [=] {
        if (!effect->isPlaying()) {
            d->activeEffects.removeAll(effect);
            effect->deleteLater();
        }
    });
    d->activeEffects.append(effect);
}

void MusicEngine::setMuteEffects(bool mute)
{
    ensureInstance();

    d->muteEffects = mute;
}

bool MusicEngine::isEffectsMuted()
{
    ensureInstance();

    return d->muteEffects;
}

MusicEngine::MusicEngine(QObject *parent) : QObject(parent)
{
    d->backgroundMusic = new QMediaPlayer();
    connect(d->backgroundMusic, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            //Loop the media
            d->backgroundMusic->setPosition(0);
            d->backgroundMusic->play();
        }
    });
    connect(d->backgroundMusic, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, [=](QMediaPlayer::Error error) {
        qDebug() << "Qt Multimedia Error" << error;
        tryNextBackgroundTrack();
    });
}

void MusicEngine::ensureInstance()
{
    if (d->instance == nullptr) d->instance = new MusicEngine();
}

void MusicEngine::tryNextBackgroundTrack()
{
    if (d->backgroundMusicUrls.count() > 0) {
        QUrl fileUrl = d->backgroundMusicUrls.dequeue();
        qDebug() << "Background music: Trying" << fileUrl;
        d->backgroundMusic->setMedia(QMediaContent(fileUrl));
        if (d->playingBackgroudMusic) playBackgroundMusic();
    }
}
