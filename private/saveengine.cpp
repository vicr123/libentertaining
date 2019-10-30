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
#include "saveengine.h"

#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QDateTime>
#include <QDataStream>

struct SaveEnginePrivate {
    SaveEngine* instance = new SaveEngine();
};

SaveEnginePrivate* SaveEngine::d = new SaveEnginePrivate();

SaveEngine::SaveEngine(QObject *parent) : QObject(parent)
{

}

SaveObjectList SaveEngine::getSaves()
{
    SaveObjectList saves;

    QDir savePath(saveDirPath());
    for (QString fileName : savePath.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time)) {
        saves.append(getSaveByFilename(QByteArray::fromHex(fileName.toUtf8())));
    }
    return saves;
}

SaveObject SaveEngine::getSaveByFilename(QString filename)
{
    SaveObject save;
    save.fileName = filename;

    QFileInfo file = save.getFileInfo();
    QIODevice* device = save.getStream();

    QDataStream stream(device);
    uint magicNumber;
    int version;
    stream >> magicNumber >> version;

    if (magicNumber == SAVE_FILE_MAGIC_NUMBER) {
        stream.setVersion(version);

        QString appName;
        stream >> appName;

        stream >> save.metadata;
    } else {
        save.metadata.insert("description", "[" + tr("Corrupt Save") + "]");
    }

    save.metadata.insert("name", filename);
    save.metadata.insert("date", file.birthTime());

    device->deleteLater();
    return save;
}

QString SaveEngine::saveDirPath()
{
    QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).absoluteFilePath("save");
    if (!QDir(savePath).exists()) {
        QDir::root().mkpath(savePath);
    }
    return savePath;
}

QIODevice* SaveObject::getStream()
{
    QFile* f = new QFile(QDir(SaveEngine::saveDirPath()).absoluteFilePath(this->fileName.toUtf8().toHex()));
    f->open(QFile::ReadWrite);
    return f;
}

QFileInfo SaveObject::getFileInfo()
{
    QFileInfo file(QDir(SaveEngine::saveDirPath()).absoluteFilePath(this->fileName.toUtf8().toHex()));
    if (!file.exists()) {
        QFile touch(file.filePath());
        touch.open(QFile::ReadWrite);
        touch.close();
    }

    return file;
}
