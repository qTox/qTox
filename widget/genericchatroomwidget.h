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

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

class GenericChatroomWidget : public QWidget
{
    Q_OBJECT
public:
    GenericChatroomWidget(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);
    void enterEvent(QEvent *);

    virtual void setAsActiveChatroom(){;}
    virtual void setAsInactiveChatroom(){;}
    virtual void updateStatusLight(){;}
    int isActive();

signals:
    void chatroomWidgetClicked(GenericChatroomWidget* widget);

public slots:

protected:
    int isActiveWidget;
    QColor lastColor;
    QHBoxLayout layout;
    QVBoxLayout textLayout;
};

#endif // GENERICCHATROOMWIDGET_H
