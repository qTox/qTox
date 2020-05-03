/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#pragma once

#include "genericchatitemwidget.h"

class CroppingLabel;
class MaskablePixmapWidget;
class QVBoxLayout;
class QHBoxLayout;
class ContentLayout;
class Friend;
class Group;
class Contact;
class GenericChatroomWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    explicit GenericChatroomWidget(bool compact, QWidget* parent = nullptr);

public slots:
    virtual void setAsActiveChatroom() = 0;
    virtual void setAsInactiveChatroom() = 0;
    virtual void updateStatusLight() = 0;
    virtual void resetEventFlags() = 0;
    virtual QString getStatusString() const = 0;
    virtual const Contact* getContact() const = 0;
    virtual const Friend* getFriend() const
    {
        return nullptr;
    }
    virtual Group* getGroup() const
    {
        return nullptr;
    }

    bool eventFilter(QObject*, QEvent*) final;

    bool isActive();

    void setName(const QString& name);
    void setStatusMsg(const QString& status);
    QString getStatusMsg() const;
    QString getTitle() const;

    void reloadTheme();

    void activate();
    void compactChange(bool compact);

signals:
    void chatroomWidgetClicked(GenericChatroomWidget* widget);
    void newWindowOpened(GenericChatroomWidget* widget);
    void middleMouseClicked();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void setActive(bool active);

protected:
    QPoint dragStartPos;
    QColor lastColor;
    QHBoxLayout* mainLayout = nullptr;
    QVBoxLayout* textLayout = nullptr;
    MaskablePixmapWidget* avatar;
    CroppingLabel* statusMessageLabel;
    bool active;
};
