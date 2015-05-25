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

int ChatLineContent::getRow() const
{
    return row;
}

int ChatLineContent::type() const
{
    return GraphicsItemType::ChatLineContentType;
}

void ChatLineContent::selectionMouseMove(QPointF)
{

}

void ChatLineContent::selectionStarted(QPointF)
{

}

void ChatLineContent::selectionCleared()
{

}

void ChatLineContent::selectionDoubleClick(QPointF)
{

}

void ChatLineContent::selectionFocusChanged(bool)
{

}

bool ChatLineContent::isOverSelection(QPointF) const
{
    return false;
}

QString ChatLineContent::getSelectedText() const
{
    return QString();
}

qreal ChatLineContent::getAscent() const
{
    return 0.0;
}

void ChatLineContent::visibilityChanged(bool)
{

}

QString ChatLineContent::getText() const
{
    return QString();
}
