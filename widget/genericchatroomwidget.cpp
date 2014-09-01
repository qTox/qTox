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

#include "genericchatroomwidget.h"
#include <QMouseEvent>

GenericChatroomWidget::GenericChatroomWidget(QWidget *parent) :
    QWidget(parent)
{
}

int GenericChatroomWidget::isActive()
{
    return isActiveWidget;
}

void GenericChatroomWidget::mousePressEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) == Qt::LeftButton)
    {
        if (isActive())
        {
            QPalette pal;
            pal.setColor(QPalette::Background, QColor(250,250,250,255));
            this->setPalette(pal);
        }
        else
        {
            QPalette pal;
            pal.setColor(QPalette::Background, QColor(85,85,85,255));
            this->setPalette(pal);
        }
    }
}

void GenericChatroomWidget::leaveEvent(QEvent *)
{
    if (isActive() != 1)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, lastColor);
        this->setPalette(pal);
    }
}

void GenericChatroomWidget::enterEvent(QEvent *)
{
    if (isActive() != 1)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, QColor(75,75,75,255));
        lastColor = this->palette().background().color();
        this->setPalette(pal);
    }
}
