/*
    Copyright © 2015-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GENERICCHATITEMWIDGET_H
#define GENERICCHATITEMWIDGET_H

#include <QFrame>
#include <QLabel>

class CroppingLabel;

class GenericChatItemWidget : public QFrame
{
    Q_OBJECT
public:
    enum ItemType
    {
        GroupItem,
        FriendOfflineItem,
        FriendOnlineItem
    };

    explicit GenericChatItemWidget(bool compact, QWidget* parent = nullptr);

    bool isCompact() const;
    void setCompact(bool compact);

    QString getName() const;

    void searchName(const QString& searchString, bool hideAll);

    Q_PROPERTY(bool compact READ isCompact WRITE setCompact)

protected:
    CroppingLabel* nameLabel;
    QLabel statusPic;

private:
    bool compact;
};

#endif // GENERICCHATITEMWIDGET_H
