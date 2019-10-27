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
#include <QPainter>
#include <QDateTime>
#include <the-libs_global.h>

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
            case Qt::UserRole + 1:
                return save.metadata.value("description");
            case Qt::UserRole + 2:
                return save.metadata.value("date");
        }
    }

    return QVariant();
}

void SavesModel::setShowNewFile(bool showNewFile)
{
    d->showNewFile = showNewFile;
}

SavesDelegate::SavesDelegate(QObject*parent) : QStyledItemDelegate(parent)
{

}

SavesDelegate::~SavesDelegate()
{

}

void SavesDelegate::paint(QPainter*painter, const QStyleOptionViewItem&option, const QModelIndex&index) const
{
    if (index.data(Qt::UserRole).type() == QVariant::Bool) {
        QStyledItemDelegate::paint(painter, option, index);
    } else {
        QLocale locale;

        QPen textPen;
        if (option.state & QStyle::State_Selected) {
            painter->setBrush(option.palette.brush(QPalette::Highlight));
        } else if (option.state & QStyle::State_MouseOver) {
            QColor col = option.palette.color(QPalette::Highlight);
            col.setAlpha(127);
            painter->setBrush(col);
        } else {
            painter->setBrush(Qt::transparent);
        }
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect);

        painter->setPen(option.palette.color(QPalette::WindowText));

        QRect textRect;
        textRect.setHeight(option.fontMetrics.height());
        textRect.setWidth(option.rect.width() - SC_DPI(12));
        textRect.moveLeft(option.rect.left() + SC_DPI(6));
        textRect.moveTop(option.rect.top() + SC_DPI(6));

        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, index.data(Qt::DisplayRole).toString());

        textRect.moveTop(textRect.bottom() + SC_DPI(3));
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, index.data(Qt::UserRole + 1).toString());

        textRect.moveTop(textRect.bottom() + SC_DPI(3));
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, locale.toString(index.data(Qt::UserRole + 2).toDateTime(), QLocale::LongFormat));
    }
}

QSize SavesDelegate::sizeHint(const QStyleOptionViewItem&option, const QModelIndex&index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (index.data(Qt::UserRole).type() != QVariant::Bool) {
        size.setHeight(option.fontMetrics.height() * 3 + SC_DPI(18));
    }
    return size;
}
