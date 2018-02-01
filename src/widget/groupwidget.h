/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "genericchatroomwidget.h"

class GroupWidget final : public GenericChatroomWidget
{
    Q_OBJECT
public:
    GroupWidget(int GroupId, const QString& Name, bool compact);
    ~GroupWidget();
    void setAsInactiveChatroom() final override;
    void setAsActiveChatroom() final override;
    void updateStatusLight() final override;
    void resetEventFlags() final override;
    QString getStatusString() const final override;
    Group* getGroup() const final override;
    void setName(const QString& name);
    void editName();

signals:
    void groupWidgetClicked(GroupWidget* widget);
    void removeGroup(int groupId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final override;
    void mousePressEvent(QMouseEvent* event) final override;
    void mouseMoveEvent(QMouseEvent* event) final override;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dragLeaveEvent(QDragLeaveEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;

private slots:
    void retranslateUi();
    void setTitle(const QString& newName);
    void updateTitle(uint32_t groupId, const QString& author, const QString& newName);
    void updateUserCount();

public:
    int groupId;
};

#endif // GROUPWIDGET_H
