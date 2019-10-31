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
#include "gamepadmodel.h"

#include <QGamepad>
#include <QGamepadManager>

GamepadModel::GamepadModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, this, [=] {
        emit dataChanged(index(0), index(rowCount()));
    });
}

int GamepadModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return QGamepadManager::instance()->connectedGamepads().count();
}

QVariant GamepadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QGamepad gamepad(QGamepadManager::instance()->connectedGamepads().at(index.row()));
    switch (role) {
        case Qt::DisplayRole:
            return gamepad.name();
        case Qt::UserRole:
            return gamepad.deviceId();
    }

    return QVariant();
}
