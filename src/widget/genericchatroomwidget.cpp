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

#include "genericchatroomwidget.h"
#include "src/widget/style.h"
#include "src/persistence/settings.h"
#include "maskablepixmapwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include <QMouseEvent>

GenericChatroomWidget::GenericChatroomWidget(QWidget *parent)
    : QFrame(parent), compact{Settings::getInstance().getCompactLayout()},
      active{false}
{
    // avatar
    if (compact)
        avatar = new MaskablePixmapWidget(this, QSize(20,20), ":/img/avatar_mask.svg");
    else
        avatar = new MaskablePixmapWidget(this, QSize(40,40), ":/img/avatar_mask.svg");

    // status text
    statusMessageLabel = new CroppingLabel(this);
    statusMessageLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setForegroundRole(QPalette::WindowText);

    // name text
    nameLabel = new CroppingLabel(this);
    nameLabel->setTextFormat(Qt::PlainText);
    nameLabel->setForegroundRole(QPalette::WindowText);

    setAutoFillBackground(true);
    reloadTheme();
    setCompact(compact);
}

void GenericChatroomWidget::setCompact(bool _compact)
{
    compact = _compact;

    delete textLayout; // has to be first, deleted by layout
    delete layout;

    compact = _compact;

    layout = new QHBoxLayout;
    textLayout = new QVBoxLayout;

    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);
    textLayout->setSpacing(0);
    textLayout->setMargin(0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    // avatar
    if (compact)
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
        statusMessageLabel->setFont(Style::getFont(Style::Small));
        nameLabel->setFont(Style::getFont(Style::Medium));
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
        statusMessageLabel->setFont(Style::getFont(Style::Medium));
        nameLabel->setFont(Style::getFont(Style::Big));
    }
}

bool GenericChatroomWidget::isActive()
{
    return active;
}

void GenericChatroomWidget::setActive(bool _active)
{
    active = _active;
    if (active)
    {
        setBackgroundRole(QPalette::Light);
        statusMessageLabel->setForegroundRole(QPalette::HighlightedText);
        nameLabel->setForegroundRole(QPalette::HighlightedText);
    }
    else
    {
        setBackgroundRole(QPalette::Window);
        statusMessageLabel->setForegroundRole(QPalette::WindowText);
        nameLabel->setForegroundRole(QPalette::WindowText);
    }
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

void GenericChatroomWidget::reloadTheme()
{
    QPalette p;

    p = statusMessageLabel->palette();
    p.setColor(QPalette::WindowText, Style::getColor(Style::LightGrey)); // Base color
    p.setColor(QPalette::HighlightedText, Style::getColor(Style::MediumGrey)); // Color when active
    statusMessageLabel->setPalette(p);

    p = nameLabel->palette();
    p.setColor(QPalette::WindowText, Style::getColor(Style::White)); // Base color
    p.setColor(QPalette::HighlightedText, Style::getColor(Style::DarkGrey)); // Color when active
    nameLabel->setPalette(p);

    p = palette();
    p.setColor(QPalette::Window, Style::getColor(Style::ThemeMedium)); // Base background color
    p.setColor(QPalette::Highlight, Style::getColor(Style::ThemeLight)); // On mouse over
    p.setColor(QPalette::Light, Style::getColor(Style::White)); // When active
    setPalette(p);
}

bool GenericChatroomWidget::isCompact() const
{
    return compact;
}

void GenericChatroomWidget::mousePressEvent(QMouseEvent* event)
{
    if (!active && event->button() == Qt::RightButton)
        setBackgroundRole(QPalette::Window);
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent*)
{
    emit chatroomWidgetClicked(this);
}

void GenericChatroomWidget::enterEvent(QEvent*)
{
    if (!active)
        setBackgroundRole(QPalette::Highlight);
}

void GenericChatroomWidget::leaveEvent(QEvent*)
{
    if (!active)
        setBackgroundRole(QPalette::Window);
}
