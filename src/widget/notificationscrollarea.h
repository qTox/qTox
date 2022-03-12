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

#pragma once

#include "tool/adjustingscrollarea.h"
#include <QHash>

class GenericChatroomWidget;
class NotificationEdgeWidget;
class Settings;
class Style;

class NotificationScrollArea final : public AdjustingScrollArea
{
public:
    explicit NotificationScrollArea(QWidget* parent = nullptr);

public slots:
    void trackWidget(Settings& settings, Style& style, GenericChatroomWidget* widget);
    void updateVisualTracking();
    void updateTracking(GenericChatroomWidget* widget);

protected:
    void resizeEvent(QResizeEvent* event) final;

private slots:
    void findNextWidget();
    void findPreviousWidget();

private:
    enum Visibility : uint8_t
    {
        Visible,
        Above,
        Below
    };
    Visibility widgetVisible(QWidget* widget) const;
    void recalculateTopEdge();
    void recalculateBottomEdge();

    QHash<GenericChatroomWidget*, Visibility> trackedWidgets;
    NotificationEdgeWidget* topEdge = nullptr;
    NotificationEdgeWidget* bottomEdge = nullptr;
    size_t referencesAbove = 0;
    size_t referencesBelow = 0;
};
