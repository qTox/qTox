/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "chatlinecontent.h"

void ChatLineContent::setIndex(int r, int c)
{
    row = r;
    col = c;
}

int ChatLineContent::getColumn() const
{
    return col;
}

int ChatLineContent::type() const
{
    return GraphicsItemType::ChatLineContentType;
}

void ChatLineContent::selectionMouseMove(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionStarted(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionCleared()
{
}

void ChatLineContent::selectionDoubleClick(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionTripleClick(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionFocusChanged(bool focusIn)
{
    std::ignore = focusIn;
}

bool ChatLineContent::isOverSelection(QPointF scenePos) const
{
    std::ignore = scenePos;
    return false;
}

QString ChatLineContent::getSelectedText() const
{
    return QString();
}

void ChatLineContent::fontChanged(const QFont& font)
{
    std::ignore = font;
}

qreal ChatLineContent::getAscent() const
{
    return 0.0;
}

void ChatLineContent::visibilityChanged(bool visible)
{
    std::ignore = visible;
}

void ChatLineContent::reloadTheme()
{
}

QString ChatLineContent::getText() const
{
    return QString();
}
