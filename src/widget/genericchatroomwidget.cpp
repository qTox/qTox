/*
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

    // avatar
    if (property("compact").toBool())
    {
        avatar = new MaskablePixmapWidget(this, QSize(20,20), ":/img/avatar_mask.svg");
    }
    else
    {
        avatar = new MaskablePixmapWidget(this, QSize(40,40), ":/img/avatar_mask.svg");
    }

    // status text
    statusMessageLabel = new CroppingLabel(this);
    statusMessageLabel->setObjectName("status");

    // name text
    nameLabel = new CroppingLabel(this);
    nameLabel->setObjectName("name");
    nameLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setTextFormat(Qt::PlainText);

    onCompactChanged(property("compact").toBool());

    setProperty("active", false);
    setStyleSheet(Style::getStylesheet(":/ui/chatroomWidgets/genericChatroomWidget.css"));
}

void GenericChatroomWidget::onCompactChanged(bool _compact)
{
    delete textLayout; // has to be first, deleted by layout
    delete layout;

    setProperty("compact", _compact);

    layout = new QHBoxLayout;
    textLayout = new QVBoxLayout;

    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);
    textLayout->setSpacing(0);
    textLayout->setMargin(0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    // avatar
    if (property("compact").toBool())
    {
        setFixedHeight(25);
        avatar->setSize(QSize(20,20));
        layout->addSpacing(18);
        layout->addWidget(avatar);
        layout->addSpacing(5);
        layout->addWidget(nameLabel);
        layout->addWidget(statusMessageLabel);
        layout->addSpacing(5);
        layout->addWidget(&statusPic);
        layout->addSpacing(5);
        layout->activate();
    }
    else
    {
        setFixedHeight(55);
        avatar->setSize(QSize(40,40));
        textLayout->addStretch();
        textLayout->addWidget(nameLabel);
        textLayout->addWidget(statusMessageLabel);
        textLayout->addStretch();
        layout->addSpacing(20);
        layout->addWidget(avatar);
        layout->addSpacing(10);
        layout->addLayout(textLayout);
        layout->addSpacing(10);
        layout->addWidget(&statusPic);
        layout->addSpacing(10);
        layout->activate();
    }

    Style::repolish(this);
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
