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

#include "chatline.h"
#include "chatlinecontent.h"

#include <QGraphicsScene>
#include "src/persistence/settings.h"

ChatLine::ChatLine()
{

}

ChatLine::~ChatLine()
{
    for (ChatLineContent* c : content)
    {
        if (c->scene())
            c->scene()->removeItem(c);

        delete c;
    }
}

void ChatLine::setRow(int idx)
{
    row = idx;

    for (int c = 0; c < static_cast<int>(content.size()); ++c)
        content[c]->setIndex(row, c);
}

void ChatLine::visibilityChanged(bool visible)
{
    if (isVisible != visible)
    {
        for (ChatLineContent* c : content)
            c->visibilityChanged(visible);
    }

    isVisible = visible;
}

int ChatLine::getRow() const
{
    return row;
}

ChatLineContent *ChatLine::getContent(int col) const
{
    if (col < static_cast<int>(content.size()) && col >= 0)
        return content[col];

    return nullptr;
}

ChatLineContent *ChatLine::getContent(QPointF scenePos) const
{
    for (ChatLineContent* c: content)
    {
        if (c->sceneBoundingRect().contains(scenePos))
            return c;
    }

    return nullptr;
}

void ChatLine::removeFromScene()
{
    for (ChatLineContent* c : content)
    {
        if (c->scene())
            c->scene()->removeItem(c);
    }
}

void ChatLine::addToScene(QGraphicsScene *scene)
{
    if (!scene)
        return;

    for (ChatLineContent* c : content)
        scene->addItem(c);
}

void ChatLine::setVisible(bool visible)
{
    for (ChatLineContent* c : content)
        c->setVisible(visible);
}

void ChatLine::selectionCleared()
{
    for (ChatLineContent* c : content)
        c->selectionCleared();
}

void ChatLine::selectionFocusChanged(bool focusIn)
{
    for (ChatLineContent* c : content)
        c->selectionFocusChanged(focusIn);
}

bool ChatLine::selectNext(const QString& text)
{
    bool done = false;

    // First, check if a selection has been made. If it has, then move to next one.
    for (ChatLineContent* c : content)
    {
        if (c->hasSelection())
        {
            done = true;

            if (c->selectNext(text))
                return true;
            else
                continue;
        }
    }

    // If not, then find the next one starting from the beginning.
    if (!done)
    {
        for (ChatLineContent* c : content)
        {
             if (c->selectNext(text))
                return true;
            else
                continue;
        }
    }

    // Text not found for selection.
    return false;
}

int ChatLine::getColumnCount()
{
    return content.size();
}

void ChatLine::updateBBox()
{
    bbox.setHeight(0);
    bbox.setWidth(width);

    for (ChatLineContent* c : content)
        bbox.setHeight(qMax(c->sceneBoundingRect().top() - bbox.top() + c->sceneBoundingRect().height(), bbox.height()));
}

QRectF ChatLine::sceneBoundingRect() const
{
    return bbox;
}

void ChatLine::addColumn(ChatLineContent* item, ColumnFormat fmt)
{
    if (!item)
        return;

    format.push_back(fmt);
    content.push_back(item);
}

void ChatLine::replaceContent(int col, ChatLineContent *lineContent)
{
    if (col >= 0 && col < static_cast<int>(content.size()) && lineContent)
    {
        QGraphicsScene* scene = content[col]->scene();
        delete content[col];

        content[col] = lineContent;
        lineContent->setIndex(row, col);

        if (scene)
            scene->addItem(content[col]);

        layout(width, bbox.topLeft());
        content[col]->visibilityChanged(isVisible);
        content[col]->update();
    }
}

void ChatLine::layout(qreal w, QPointF scenePos)
{
    width = w;
    bbox.setTopLeft(scenePos);

    qreal fixedWidth = (content.size()-1) * columnSpacing;
    qreal varWidth = 0.0; // used for normalisation

    for (int i = 0; i < static_cast<int>(format.size()); ++i)
    {
        switch(format[i].policy)
        {
            case ColumnFormat::RightColumn:
                format[i].size = Settings::getInstance().getColumnRightWidth();
                fixedWidth += format[i].size;
                break;
            case ColumnFormat::LeftColumn:
                format[i].size = Settings::getInstance().getColumnLeftWidth();
                fixedWidth += format[i].size;
                break;
            case ColumnFormat::FixedSize:
                fixedWidth += format[i].size;
                break;
            case ColumnFormat::VariableSize:
                varWidth += format[i].size;
                break;
            default:
                break;
        }
    }

    if (varWidth == 0.0)
        varWidth = 1.0;

    qreal leftover = qMax(0.0, width - fixedWidth);

    qreal maxVOffset = 0.0;
    qreal xOffset = 0.0;
    qreal xPos[content.size()];


    for (int i = 0; i < static_cast<int>(content.size()); ++i)
    {
        // calculate the effective width of the current column
        qreal width;

        switch(format[i].policy)
        {
            case ColumnFormat::RightColumn:
            case ColumnFormat::LeftColumn:
            case ColumnFormat::FixedSize:
                width = format[i].size;
                break;
            case ColumnFormat::VariableSize:
                width = format[i].size / varWidth * leftover;
                break;
            default:
                width = 0;
        }

        // set the width of the current column
        content[i]->setWidth(width);

        // calculate horizontal alignment
        qreal xAlign = 0.0;

        switch(format[i].hAlign)
        {
            case ColumnFormat::Right:
                xAlign = width - content[i]->boundingRect().width();
                break;
            case ColumnFormat::Center:
                xAlign = (width - content[i]->boundingRect().width()) / 2.0;
                break;
            default:
                break;
        }

        // reposition
        xPos[i] = scenePos.x() + xOffset + xAlign;

        xOffset += width + columnSpacing;
        maxVOffset = qMax(maxVOffset, content[i]->getAscent());
    }

    for (int i = 0; i < static_cast<int>(content.size()); ++i)
    {
        // calculate vertical alignment
        // vertical alignment may depend on width, so we do it in a second pass
        qreal yOffset = maxVOffset - content[i]->getAscent();

        // reposition
        content[i]->setPos(xPos[i], scenePos.y() + yOffset);
    }

    updateBBox();
}

void ChatLine::moveBy(qreal deltaY)
{
    // reposition only
    for (ChatLineContent* c : content)
        c->moveBy(0, deltaY);

    bbox.moveTop(bbox.top() + deltaY);
}

bool ChatLine::lessThanBSRectTop(const ChatLine::Ptr& lhs, const qreal& rhs)
{
    return lhs->sceneBoundingRect().top() < rhs;
}

bool ChatLine::lessThanBSRectBottom(const ChatLine::Ptr& lhs, const qreal& rhs)
{
    return lhs->sceneBoundingRect().bottom() < rhs;
}

bool ChatLine::lessThanRowIndex(const ChatLine::Ptr& lhs, const ChatLine::Ptr& rhs)
{
    return lhs->getRow() < rhs->getRow();
}
