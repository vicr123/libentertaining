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
#ifndef SAVEENGINE_H
#define SAVEENGINE_H

#include <QObject>
#include <QVariantMap>

#define SAVE_FILE_MAGIC_NUMBER 0xE3928472

class QIODevice;
class QFileInfo;
struct SaveObject {
    QString fileName;
    QVariantMap metadata;

    QIODevice* getStream();
    QFileInfo getFileInfo();
};
typedef QList<SaveObject> SaveObjectList;

struct SaveEnginePrivate;
class SaveEngine : public QObject
{
        Q_OBJECT
    public:
        explicit SaveEngine(QObject *parent = nullptr);

        static SaveObjectList getSaves();
        static SaveObject getSaveByFilename(QString filename);

    signals:

    public slots:

    protected:
        friend SaveObject;
        static QString saveDirPath();

    private:
        static SaveEnginePrivate* d;
};

#endif // SAVEENGINE_H
