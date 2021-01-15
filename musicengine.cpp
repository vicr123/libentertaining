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
#include "music/musicelementplayer.h"
#include "music/filemusicelement.h"
#include "music/groupmusicelement.h"

struct MusicEnginePrivate {
    MusicEngine* instance = nullptr;

    AbstractMusicElement* backgroundMusic = nullptr;
    MusicElementPlayer* backgroundMusicPlayer = nullptr;
    qreal backgroundVolume;
    QString backgroundInitialResource, backgroundLoopingResource;

    QList<QSoundEffect*> activeEffects;
    bool muteEffects = false;

    QMap<QString, QList<MusicEngine::MusicStream>> multiTrackStreams;

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
    setBackgroundMusic(QUrl(), path);
}

void MusicEngine::setBackgroundMusic(QString audioResource) {
    if (d->multiTrackStreams.contains(audioResource)) {
        if (d->backgroundLoopingResource == audioResource) return;

        if (d->backgroundMusicPlayer) d->backgroundMusicPlayer->deleteLater();
        if (d->backgroundMusic) d->backgroundMusic->deleteLater();

        //Set up the music element
        GroupMusicElement* groupElement = new GroupMusicElement();
        for (MusicEngine::MusicStream stream : d->multiTrackStreams.value(audioResource)) {
            FileMusicElement* element = new FileMusicElement(stream.streamName, stream.initialPaths, stream.loopingPaths);
            groupElement->giveElement(element);
        }
//        d->backgroundMusic = new FileMusicElement("bgm", initialResource, loopingResource);
        d->backgroundMusic = groupElement;
        d->backgroundMusicPlayer = new MusicElementPlayer(d->backgroundMusic);
        d->backgroundMusicPlayer->setVolume(d->backgroundVolume);
        d->backgroundMusicPlayer->play();

        d->backgroundInitialResource = "";
        d->backgroundLoopingResource = audioResource;
    } else {
        setBackgroundMusic("", audioResource);
    }
}

void MusicEngine::setBackgroundMusic(QUrl initialPath, QUrl loopingPath) {
    ensureInstance();

    if (d->backgroundMusicPlayer) d->backgroundMusicPlayer->deleteLater();
    if (d->backgroundMusic) d->backgroundMusic->deleteLater();

    d->backgroundMusic = new FileMusicElement("bgm", {initialPath}, {loopingPath});
    d->backgroundMusicPlayer = new MusicElementPlayer(d->backgroundMusic);
    d->backgroundMusicPlayer->setVolume(d->backgroundVolume);
    d->backgroundMusicPlayer->play();

    d->backgroundInitialResource.clear();
    d->backgroundLoopingResource.clear();
}

void MusicEngine::setBackgroundMusic(QString initialResource, QString loopingResource) {
    ensureInstance();

    if (d->backgroundInitialResource == initialResource && d->backgroundLoopingResource == loopingResource) return;

    if (d->backgroundMusicPlayer) d->backgroundMusicPlayer->deleteLater();
    if (d->backgroundMusic) d->backgroundMusic->deleteLater();

    d->backgroundMusic = new FileMusicElement("bgm", initialResource, loopingResource);
    d->backgroundMusicPlayer = new MusicElementPlayer(d->backgroundMusic);
    d->backgroundMusicPlayer->setVolume(d->backgroundVolume);
    d->backgroundMusicPlayer->play();

    d->backgroundInitialResource = initialResource;
    d->backgroundLoopingResource = loopingResource;
}

void MusicEngine::playBackgroundMusic() {
    ensureInstance();

    if (d->backgroundMusicPlayer) d->backgroundMusicPlayer->play();
}

void MusicEngine::pauseBackgroundMusic() {
    ensureInstance();

    if (d->backgroundMusicPlayer) d->backgroundMusicPlayer->pause();
}

void MusicEngine::setUserBackgroundVolume(qreal volume) {
    ensureInstance();

    d->backgroundVolume = volume;
    if (d->backgroundMusicPlayer) d->backgroundMusicPlayer->setVolume(volume);
}

qreal MusicEngine::userBackgroundVolume() {
    ensureInstance();

    return d->backgroundVolume;
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

MusicEngine::MusicStream MusicEngine::createMusicStream(QString streamName, QUrl initialPath, QUrl loopingPath) {
    MusicStream stream;
    stream.streamName = streamName;
    stream.initialPaths = {initialPath};
    stream.loopingPaths = {loopingPath};
    return stream;
}

MusicEngine::MusicStream MusicEngine::createMusicStream(QString streamName, QString initialResource, QString loopingResource) {
    MusicStream stream;
    stream.streamName = streamName;
    stream.initialPaths = resolveAudioResource(initialResource);
    stream.loopingPaths = resolveAudioResource(loopingResource);
    return stream;
}

void MusicEngine::registerMultiTrackResource(QString resourceName, QList<MusicEngine::MusicStream> streams) {
    d->multiTrackStreams.insert(resourceName, streams);
}

void MusicEngine::setStreamVolume(QString streamName, qreal volume) {
    //TODO: save the stream volume so we can restore it later
    if (d->backgroundMusic) d->backgroundMusic->setStreamVolume(streamName, volume);
}

MusicEngine::MusicEngine(QObject* parent) : QObject(parent) {

}

void MusicEngine::ensureInstance() {
    if (d->instance == nullptr) d->instance = new MusicEngine();
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
