/*
    Copyright © 2014-2015 by The qTox Project

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

#ifndef GENERICCHATROOMWIDGET_H
#define GENERICCHATROOMWIDGET_H

#include "genericchatitemwidget.h"

class CroppingLabel;
class MaskablePixmapWidget;
class QVBoxLayout;
class QHBoxLayout;
class ContentLayout;
class Friend;
class Group;

class GenericChatroomWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    enum class Type { Friend = 0, Group = 1 };

public:
    explicit GenericChatroomWidget(QWidget *parent = 0);

    virtual Type type() const = 0;
    virtual void setAsActiveChatroom(bool activate) = 0;
    virtual void updateStatusLight() = 0;
    virtual bool chatFormIsSet(bool focus) const = 0;
    virtual void resetEventFlags() = 0;
    virtual QString getTitle() const  = 0;

    bool eventFilter(QObject *, QEvent *) final;

    bool isActive();
    void setActive(bool active);

    void setName(const QString& name);
    void setStatusMsg(const QString& status);
    QString getStatusMsg() const;

	void reloadTheme();

    void setEventFlag(bool enabled);
    inline bool getEventFlag() const
    {
        return hasNewMessages;
    }

public slots:
    void onCompactLayoutChanged(bool compact);

signals:
    void chatroomWidgetClicked(GenericChatroomWidget* widget, bool group = false);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;

protected:
    QPoint dragStartPos;
    QColor lastColor;
    QHBoxLayout* mainLayout = nullptr;
    MaskablePixmapWidget* avatar;
    CroppingLabel* statusMessageLabel;
	bool active;
    bool hasNewMessages;
};

#endif // GENERICCHATROOMWIDGET_H
