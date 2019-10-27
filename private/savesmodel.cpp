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
#include "savesmodel.h"

#include "saveengine.h"
#include <QIcon>

struct SavesModelPrivate {
    SaveObjectList saves;
    bool showNewFile = true;
};

SavesModel::SavesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new SavesModelPrivate();
    d->saves = SaveEngine::getSaves();
}

SavesModel::~SavesModel()
{
    delete d;
}

int SavesModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    int count = d->saves.count();
    if (d->showNewFile) count++;
    return count;
}

QVariant SavesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() == d->saves.count()) {
        switch (role) {
            case Qt::DisplayRole:
                return tr("Create New Save");
            case Qt::DecorationRole:
                return QIcon::fromTheme("list-add");
            case Qt::UserRole:
                return false;
        }
    } else {
        SaveObject save = d->saves.at(index.row());
        switch (role) {
            case Qt::DisplayRole:
                return save.metadata.value("name");
            case Qt::UserRole:
                return save.fileName;
        }
    }

    return QVariant();
}

void SavesModel::setShowNewFile(bool showNewFile)
{
    d->showNewFile = showNewFile;
}
