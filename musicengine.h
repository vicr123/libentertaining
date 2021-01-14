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
#ifndef MUSICENGINE_H
#define MUSICENGINE_H

#include "libentertaining_global.h"
#include <QObject>

struct MusicEnginePrivate;
class FileMusicElement;
class LIBENTERTAINING_EXPORT MusicEngine : public QObject {
        Q_OBJECT
    public:
        enum KnownSoundEffect {
            FocusChanged,
            FocusChangedFailed,
            Pause,
            Selection,
            Backstep,
            Warning,
            Error
        };

        static void setBackgroundMusic(QUrl path);
        static void setBackgroundMusic(QString audioResource);
        static void setBackgroundMusic(QUrl initialPath, QUrl loopingPath);
        static void setBackgroundMusic(QString initialResource, QString loopingResource);
        static void playBackgroundMusic();
        static void pauseBackgroundMusic();

        static void setUserBackgroundVolume(qreal volume);
        static qreal userBackgroundVolume();

        static void playSoundEffect(KnownSoundEffect effect);
        static void playSoundEffect(QUrl path);
        static void playSoundEffect(QString audioResource);

        static void setMuteEffects(bool mute);
        static bool isEffectsMuted();

    signals:

    public slots:

    protected:
        friend FileMusicElement;
        static QList<QUrl> resolveAudioResource(QString audioResource);

    private:
        static MusicEnginePrivate* d;

        explicit MusicEngine(QObject* parent = nullptr);
        static void ensureInstance();
};

#endif // MUSICENGINE_H
