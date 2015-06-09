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
#include <QBoxLayout>
#include <QLabel>

class CroppingLabel;
class MaskablePixmapWidget;

namespace Ui {
    class MainWindow;
}

class GenericChatroomWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    GenericChatroomWidget(QWidget *parent = 0);

    virtual void setAsActiveChatroom(){;}
    virtual void setAsInactiveChatroom(){;}
    virtual void updateStatusLight(){;}
    virtual void setChatForm(Ui::MainWindow &){;}
    virtual void resetEventFlags(){;}
    virtual QString getStatusString(){return QString::null;}

    bool isActive();
    void setActive(bool active);

    void setName(const QString& name);
    void setStatusMsg(const QString& status);

    QString getStatusMsg() const;

    void reloadTheme();

    bool isCompact() const;

public slots:
    void setCompact(bool compact);

signals:
    void chatroomWidgetClicked(GenericChatroomWidget* widget);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent (QMouseEvent* event) override;
    virtual void enterEvent(QEvent* e) override;
    virtual void leaveEvent(QEvent* e) override;

protected:
    QColor lastColor;
    QHBoxLayout* mainLayout = nullptr;
    QVBoxLayout* textLayout = nullptr;
    MaskablePixmapWidget* avatar;
    CroppingLabel* statusMessageLabel;
    bool compact, active;
};

#endif // GENERICCHATROOMWIDGET_H
