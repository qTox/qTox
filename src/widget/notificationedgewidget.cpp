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

#include "notificationedgewidget.h"
#include "style.h"
#include "src/persistence/settings.h"
#include <QBoxLayout>
#include <QLabel>

#include <QDebug>

NotificationEdgeWidget::NotificationEdgeWidget(Position position, Settings& settings,
    Style& style, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground); // Show background.
    setStyleSheet(style.getStylesheet("notificationEdge/notificationEdge.css", settings));
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addStretch();

    textLabel = new QLabel(this);
    textLabel->setMinimumHeight(textLabel->sizeHint().height()); // Prevent cut-off text.
    layout->addWidget(textLabel);

    QLabel* arrowLabel = new QLabel(this);

    if (position == Top)
        arrowLabel->setPixmap(QPixmap(style.getImagePath("chatArea/scrollBarUpArrow.svg", settings)));
    else
        arrowLabel->setPixmap(QPixmap(style.getImagePath("chatArea/scrollBarDownArrow.svg", settings)));

    layout->addWidget(arrowLabel);
    layout->addStretch();

    setCursor(Qt::PointingHandCursor);
}

void NotificationEdgeWidget::updateNotificationCount(int count)
{
    textLabel->setText(tr("Unread message(s)", "", count));
}

void NotificationEdgeWidget::mouseReleaseEvent(QMouseEvent* event)
{
    emit clicked();
    QWidget::mousePressEvent(event);
}
