/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "genericchatitemwidget.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/style.h"
#include <QVariant>

GenericChatItemWidget::GenericChatItemWidget(bool compact_, Style& style,
    QWidget* parent)
    : QFrame(parent)
    , compact(compact_)
{
    setProperty("compact", compact_);

    nameLabel = new CroppingLabel(this);
    nameLabel->setObjectName("name");
    nameLabel->setTextFormat(Qt::PlainText);

    connect(&style, &Style::themeReload, this, &GenericChatItemWidget::reloadTheme);
}

bool GenericChatItemWidget::isCompact() const
{
    return compact;
}

void GenericChatItemWidget::setCompact(bool compact_)
{
    compact = compact_;
}

QString GenericChatItemWidget::getName() const
{
    return nameLabel->fullText();
}

void GenericChatItemWidget::searchName(const QString& searchString, bool hide)
{
    setVisible(!hide && getName().contains(searchString, Qt::CaseInsensitive));
}
