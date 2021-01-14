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
#include "musicelementplayer.h"

#include <QAudioOutput>
#include <QIODevice>
#include "abstractmusicelement.h"

struct MusicElementPlayerPrivate {
    QAudioFormat format;
    AbstractMusicElement* element;
    QAudioOutput* output;
    QIODevice* sink = nullptr;

    quint64 audioPointer = 0;

    bool isPlaying = false;
    bool isSuspended = false;
};

MusicElementPlayer::MusicElementPlayer(AbstractMusicElement* element, QObject* parent) : QObject(parent) {
    d = new MusicElementPlayerPrivate();
    d->element = element;

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

    connect(element, &AbstractMusicElement::attemptBufferFill, this, &MusicElementPlayer::fillAudioBuffer);

    d->sink = d->output->start();
}

MusicElementPlayer::~MusicElementPlayer() {
    delete d;
}

void MusicElementPlayer::play() {
    if (d->isSuspended) d->output->resume();
    d->isSuspended = false;
    d->isPlaying = true;
}

void MusicElementPlayer::pause() {
    if (!d->isSuspended) d->output->suspend();
    d->isSuspended = true;
    d->isPlaying = false;
}

void MusicElementPlayer::setVolume(qreal volume) {
    d->output->setVolume(volume);
}

void MusicElementPlayer::fillAudioBuffer() {
    //Make sure the background sink is open
    if (!d->sink) return;

    if (d->element->blocking(d->output->bufferSize())) {
        if (!d->isSuspended) d->output->suspend();
        d->isSuspended = true;
        return;
    }

    quint64 free = d->output->bytesFree();
    QByteArray audioData = d->element->data(d->audioPointer, free);
    d->audioPointer += audioData.length();
    d->sink->write(audioData);

    if (d->isPlaying && d->isSuspended) {
        d->output->resume();
        d->isSuspended = false;
    }
}
