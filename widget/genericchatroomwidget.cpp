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
#include "style.h"
#include <QMouseEvent>

GenericChatroomWidget::GenericChatroomWidget(QWidget *parent)
    : QWidget(parent)
    , isActiveWidget(false)
{
}

bool GenericChatroomWidget::isActive()
{
    return isActiveWidget;
}

void GenericChatroomWidget::setActive(bool active)
{
    isActiveWidget = active;

    if (active)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, Style::getColor(Style::White));
        setPalette(pal);
    }
    else
    {
        QPalette pal;
        pal.setColor(QPalette::Background, Style::getColor(Style::MediumGrey));
        setPalette(pal);
    }
}

void GenericChatroomWidget::leaveEvent(QEvent *)
{
    if (!isActive())
    {
        QPalette pal;
        pal.setColor(QPalette::Background, Style::getColor(Style::MediumGrey));
        setPalette(pal);
    }
}

void GenericChatroomWidget::enterEvent(QEvent *)
{
    if (!isActive())
    {
        QPalette pal;
        pal.setColor(QPalette::Background, Style::getColor(Style::MediumGrey).lighter(120));
        setPalette(pal);
    }
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent*)
{
    emit chatroomWidgetClicked(this);
}
