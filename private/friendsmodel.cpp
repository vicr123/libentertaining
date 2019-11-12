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
#include "friendsmodel.h"

#include <QIcon>
#include <QPainter>
#include "online/onlineapi.h"

struct FriendsModelAction {
    QIcon icon;
    QString text;
    QString action;
    QString description;
};

struct Friend {
    QString username;
    QString status;

    bool isOnline;
    QString application;
    QString applicationDisplayName;
};

struct FriendsModelPrivate {
    QList<FriendsModelAction> actions;
    QList<Friend> friends;
};

FriendsModel::FriendsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new FriendsModelPrivate();

    d->actions.append({
        QIcon(":/libentertaining/icons/user.svg"),
        OnlineApi::getLoggedInUsername(),
        "profile",
        tr("You")
    });
    d->actions.append({
        QIcon(":/libentertaining/icons/list-add.svg"),
        tr("Add Friend"),
        "friend-add",
        tr("Add a friend to your friends list")
    });

    update();
}

FriendsModel::~FriendsModel()
{
    delete d;
}

int FriendsModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return d->actions.count() + d->friends.count() + 1;
}

QVariant FriendsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    if (row < d->actions.count()) {
        FriendsModelAction action = d->actions.at(row);
        switch (role) {
            case Qt::DisplayRole:
                return action.text;
            case Qt::DecorationRole:
                return action.icon;
            case Qt::UserRole:
                return action.action;
            case Qt::UserRole + 1:
                return action.description;
        }

        return QVariant();
    }
    row -= d->actions.count();

    if (row == 0) {
        //Seperator for friends header
        switch (role) {
            case Qt::DisplayRole:
                return tr("Friends").toUpper();
            case Qt::UserRole:
                return "sep";
        }

        return QVariant();
    }
    row -= 1;

    if (row < d->friends.count()) {
        Friend f = d->friends.at(row);
        switch (role) {
            case Qt::DisplayRole:
                return f.username;
            case Qt::UserRole:
                return "friend";
            case Qt::DecorationRole:
                return QIcon(":/libentertaining/icons/user.svg");
            case Qt::UserRole + 1: {
                QString status = f.status;
                if (status == "friend") {
                    if (f.isOnline) {
                        return f.applicationDisplayName;
                    } else {
                        return tr("Offline");
                    }
                } else if (status == "request-incoming") {
                    return tr("Incoming Request");
                } else if (status == "request-outgoing") {
                    return tr("Outgoing Friend Request");
                } else {
                    return tr("User");
                }
            }
            case Qt::UserRole + 2:
                return f.status;
            case Qt::UserRole + 3:
                return f.isOnline;
        }

        return QVariant();
    }

    return QVariant();
}

void FriendsModel::update()
{
    OnlineApi::instance()->get("/friends")->then([=](QJsonDocument doc) {
        d->friends.clear();

        QJsonArray array = doc.array();
        for (QJsonValue v : array) {
            QJsonObject friendObject = v.toObject();
            Friend f;
            f.username = friendObject.value("username").toString();
            f.status = friendObject.value("status").toString();
            if (f.status == "friend") {
                QJsonValue onlineValue = friendObject.value("onlineState");
                if (onlineValue.isBool()) {
                    f.isOnline = false;
                } else {
                    QJsonObject onlineStatus = onlineValue.toObject();
                    f.isOnline = true;
                    f.application = onlineStatus.value("application").toString();
                    f.applicationDisplayName = onlineStatus.value("applicationDisplayName").toString();
                }
            }
            d->friends.append(f);
        }

        emit dataChanged(index(0), index(rowCount()));
    });
}

FriendsDelegate::FriendsDelegate(QWidget *parent) : QStyledItemDelegate(parent) {

}

void FriendsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data(Qt::UserRole).toString() == "sep") {
        QRect textRect;
        textRect.setLeft(option.rect.left() + SC_DPI(9));
        textRect.setTop(option.rect.top() + SC_DPI(9));
        textRect.setBottom(option.rect.top() + option.fontMetrics.height() + SC_DPI(9));
        textRect.setRight(option.rect.right() - SC_DPI(9));

        QFont fnt = option.font;
        fnt.setBold(true);
        painter->setFont(fnt);

        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, index.data(Qt::DisplayRole).toString());
    } else {
        painter->setFont(option.font);

        QRect iconRect;
        iconRect.setLeft(option.rect.left() + SC_DPI(12));
        iconRect.setTop(option.rect.top() + SC_DPI(6));
        iconRect.setBottom(iconRect.top() + SC_DPI(32));
        iconRect.setRight(iconRect.left() + SC_DPI(32));

        QRect textRect;
        textRect.setLeft(iconRect.right() + SC_DPI(6));
        textRect.setTop(option.rect.top() + SC_DPI(6));
        textRect.setBottom(option.rect.top() + option.fontMetrics.height() + SC_DPI(6));
        textRect.setRight(option.rect.right());

        QRect descRect;
        descRect.setLeft(iconRect.right() + SC_DPI(6));
        descRect.setTop(option.rect.top() + option.fontMetrics.height() + SC_DPI(8));
        descRect.setBottom(option.rect.top() + option.fontMetrics.height() * 2 + SC_DPI(6));
        descRect.setRight(option.rect.right());

        if (option.state & QStyle::State_Selected) {
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::Highlight));
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::HighlightedText));
            painter->drawText(textRect, index.data().toString());
            painter->drawText(descRect, index.data(Qt::UserRole + 1).toString());
        } else if (option.state & QStyle::State_MouseOver) {
            QColor col = option.palette.color(QPalette::Highlight);
            col.setAlpha(127);
            painter->setBrush(col);
            painter->setPen(Qt::transparent);
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(descRect, index.data(Qt::UserRole + 1).toString());
        } else {
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(descRect, index.data(Qt::UserRole + 1).toString());
        }

        QImage icon = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconRect.size()).toImage();
        theLibsGlobal::tintImage(icon, option.palette.color(QPalette::WindowText));
        painter->drawImage(iconRect, icon);

        if (index.data(Qt::UserRole + 3).toBool()) { //isOnline
            painter->setBrush(QColor(0, 100, 0));
            painter->setPen(Qt::transparent);
            painter->drawRect(option.rect.left(), option.rect.top(), SC_DPI(6), option.rect.height());
        }
    }
}

QSize FriendsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data(Qt::UserRole).toString() == "sep") {
        int fontHeight = option.fontMetrics.height();
        return QSize(option.fontMetrics.horizontalAdvance(index.data().toString()) + SC_DPI(18), fontHeight + SC_DPI(18));
    } else {
        int fontHeight = option.fontMetrics.height() * 2 + SC_DPI(14);
        int iconHeight = SC_DPI(46);

        return QSize(option.fontMetrics.horizontalAdvance(index.data().toString()), qMax(fontHeight, iconHeight));
    }
}
