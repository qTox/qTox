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
#include "src/misc/style.h"
#include "src/misc/settings.h"
#include "maskablepixmapwidget.h"
#include "croppinglabel.h"
#include <QMouseEvent>
#include <QStyle>

GenericChatroomWidget::GenericChatroomWidget(QWidget *parent)
    : QFrame(parent)
{
    setProperty("compact", Settings::getInstance().getCompactLayout());

    if (property("compact").toBool())
    {
        setFixedHeight(25);
    }
    else
    {
        setFixedHeight(55);
    }

    setLayout(&layout);
    layout.setSpacing(0);
    layout.setMargin(0);
    textLayout.setSpacing(0);
    textLayout.setMargin(0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    // avatar
    if (property("compact").toBool())
    {
        avatar = new MaskablePixmapWidget(this, QSize(20,20), ":/img/avatar_mask.png");
    }
    else
    {
        avatar = new MaskablePixmapWidget(this, QSize(40,40), ":/img/avatar_mask.png");
    }

    // status text
    statusMessageLabel = new CroppingLabel(this);
    statusMessageLabel->setObjectName("status");
    statusMessageLabel->setClickableURLs(true);

    // name text
    nameLabel = new CroppingLabel(this);
    nameLabel->setObjectName("name");
    
    if (property("compact").toBool())
    {
        layout.addSpacing(18);
        layout.addWidget(avatar);
        layout.addSpacing(5);
        layout.addWidget(nameLabel);
        layout.addWidget(statusMessageLabel);
        layout.addSpacing(5);
        layout.addWidget(&statusPic);
        layout.addSpacing(5);
        layout.activate();
    }
    else
    {
        textLayout.addStretch();
        textLayout.addWidget(nameLabel);
        textLayout.addWidget(statusMessageLabel);
        textLayout.addStretch();

        layout.addSpacing(20);
        layout.addWidget(avatar);
        layout.addSpacing(10);
        layout.addLayout(&textLayout);
        layout.addSpacing(10);
        layout.addWidget(&statusPic);
        layout.addSpacing(10);
        layout.activate();
    }

    setProperty("active", false);
    setStyleSheet(Style::getStylesheet(":/ui/chatroomWidgets/genericChatroomWidget.css"));

    connect(statusMessageLabel, &CroppingLabel::clicked, [&](){emit chatroomWidgetClicked(this);});
}

bool GenericChatroomWidget::isActive()
{
    return property("active").toBool();
}

void GenericChatroomWidget::setActive(bool active)
{
    setProperty("active", active);
    Style::repolish(this);
}

void GenericChatroomWidget::setName(const QString &name)
{
    nameLabel->setText(name);
}

void GenericChatroomWidget::setStatusMsg(const QString &status)
{
    statusMessageLabel->setText(status);
}

QString GenericChatroomWidget::getName() const
{
    return nameLabel->fullText();
}

QString GenericChatroomWidget::getStatusMsg() const
{
    return statusMessageLabel->text();
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent*)
{
    emit chatroomWidgetClicked(this);
}

void GenericChatroomWidget::reloadTheme()
{
    setStyleSheet(Style::getStylesheet(":/ui/chatroomWidgets/genericChatroomWidget.css"));
}

bool GenericChatroomWidget::isCompact() const
{
    return compact;
}

void GenericChatroomWidget::setCompact(bool compact)
{
    this->compact = compact;
    Style::repolish(this);
}
