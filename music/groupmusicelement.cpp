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
#include "groupmusicelement.h"

#include <QByteArray>
#include <QtConcurrent>

struct GroupMusicElementPrivate {
    QList<AbstractMusicElement*> elements;
};

GroupMusicElement::GroupMusicElement(QObject* parent) : AbstractMusicElement(QStringLiteral("GroupTrack"), parent) {
    d = new GroupMusicElementPrivate();
}

GroupMusicElement::~GroupMusicElement() {
    for (AbstractMusicElement* element : qAsConst(d->elements)) {
        element->deleteLater();
    }
    delete d;
}

void GroupMusicElement::giveElement(AbstractMusicElement* element) {
    d->elements.append(element);
    connect(element, &AbstractMusicElement::attemptBufferFill, this, &GroupMusicElement::attemptBufferFill);
}

QByteArray GroupMusicElement::data(quint64 offset, quint64 length) {
    if (d->elements.isEmpty()) return QByteArray(length, 0);

    //Combine all the data into one
    QList<QByteArray> elementData;
    for (AbstractMusicElement* element : qAsConst(d->elements)) {
        elementData.append(element->data(offset, length));
    }

    //Now combine all the element data into one stream
//    auto map = [ = ](const QByteArray & data) -> QByteArray {
//        QByteArray retval;
//        for (const char& c : data) {
//            retval.append(c / d->elements.length());
//        }
//        return retval;
//    };
//    auto reduce = [ = ](QByteArray & finalData, const QByteArray & data) -> void {
//        if (finalData.isEmpty()) finalData.fill(0, length);
//        for (int i = 0; i < finalData.length(); i++) {
//            finalData.replace(i, finalData.at(i) + data.at(i));
//        }
//    };
//    QByteArray data = QtConcurrent::blockingMappedReduced<QByteArray>(elementData.begin(), elementData.end(), map, reduce);
    QByteArray data;
    for (quint64 i = 0; i < length; i++) {
        uchar sample = 0;
        for (const QByteArray& element : elementData) {
            sample += static_cast<uchar>(element.at(i)) / d->elements.length();
        }
//        finalData.replace(i, finalData.at(i) + data.at(i));
        data.append(sample);
    }

    return data;
}

bool GroupMusicElement::blocking(quint64 bufferSize) {
    for (AbstractMusicElement* element : qAsConst(d->elements)) {
        if (element->blocking(bufferSize)) return true;
    }
    return false;
}

void GroupMusicElement::setStreamVolume(QString trackName, qreal volume) {
    for (AbstractMusicElement* element : qAsConst(d->elements)) {
        element->setStreamVolume(trackName, volume);
    }
}
