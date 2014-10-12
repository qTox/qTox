/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef GENERICCHATROOMWIDGET_H
#define GENERICCHATROOMWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class CroppingLabel;
class MaskablePixmapWidget;

namespace Ui {
    class MainWindow;
}

class GenericChatroomWidget : public QFrame
{
    Q_OBJECT
public:
    GenericChatroomWidget(QWidget *parent = 0);
    void mouseReleaseEvent (QMouseEvent* event);

    virtual void setAsActiveChatroom(){;}
    virtual void setAsInactiveChatroom(){;}
    virtual void updateStatusLight(){;}
    virtual void setChatForm(Ui::MainWindow &){;}
    virtual void resetEventFlags(){;}

    bool isActive();
    void setActive(bool active);

    void setName(const QString& name);
    void setStatusMsg(const QString& status);

    QString getName() const;
    QString getStatusMsg() const;

signals:
    void chatroomWidgetClicked(GenericChatroomWidget* widget);

public slots:

protected:
    QColor lastColor;
    QHBoxLayout layout;
    QVBoxLayout textLayout;
    MaskablePixmapWidget* avatar;
    QLabel statusPic;
    CroppingLabel *nameLabel, *statusMessageLabel;
};

#endif // GENERICCHATROOMWIDGET_H
