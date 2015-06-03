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

#include "genericchatitemwidget.h"
#include "src/misc/style.h"
#include "src/misc/settings.h"
#include "croppinglabel.h"
#include <QVariant>

GenericChatItemWidget::GenericChatItemWidget(QWidget *parent)
    : QFrame(parent)
{
    setProperty("compact", Settings::getInstance().getCompactLayout());

    nameLabel = new CroppingLabel(this);
    nameLabel->setObjectName("name");
    nameLabel->setTextFormat(Qt::PlainText);
}

bool GenericChatItemWidget::isCompact() const
{
    return compact;
}

void GenericChatItemWidget::setCompact(bool compact)
{
    this->compact = compact;
    Style::repolish(this);
}

QString GenericChatItemWidget::getName() const
{
    return nameLabel->fullText();
}

bool GenericChatItemWidget::operator<(const GenericChatItemWidget& other) const
{
    int compareValue = getName().localeAwareCompare(other.getName());
    //if (compareValue == 0)
    //    return this < &other; // Consistent ordering.
    return  compareValue < 0;
}
