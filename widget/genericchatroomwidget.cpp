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
    setMouseTracking(true);
    setFixedHeight(55);

    setLayout(&layout);
    layout.setSpacing(0);
    layout.setMargin(0);
    textLayout.setSpacing(0);
    textLayout.setMargin(0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    setStyleSheet(QString("background-color: %1").arg(Style::getColor(Style::MediumGrey).name()));
}

bool GenericChatroomWidget::isActive()
{
    return isActiveWidget;
}
void GenericChatroomWidget::setActive(bool active)
{
    isActiveWidget = active;

    if (active)
        setStyleSheet(QString("background-color: %1").arg(Style::getColor(Style::White).name()));
    else
        setStyleSheet(QString("background-color: %1").arg(Style::getColor(Style::MediumGrey).name()));
}

void GenericChatroomWidget::leaveEvent(QEvent *)
{
    if (!isActive())
        setStyleSheet(QString("background-color: %1").arg(Style::getColor(Style::MediumGrey).name()));
}

void GenericChatroomWidget::enterEvent(QEvent *)
{
    if (!isActive())
        setStyleSheet(QString("background-color: %1").arg(Style::getColor(Style::MediumGrey).lighter(120).name()));
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent*)
{
    emit chatroomWidgetClicked(this);
}
