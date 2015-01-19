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

#include "chatline.h"
#include "chatlog.h"
#include "chatlinecontent.h"

#include <QTextLine>
#include <QDebug>
#include <QGraphicsScene>

#define CELL_SPACING 15

ChatLine::ChatLine()
{

}

ChatLine::~ChatLine()
{
    for(ChatLineContent* c : content)
    {
        if(c->scene())
            c->scene()->removeItem(c);

        delete c;
    }
}

void ChatLine::setRowIndex(int idx)
{
    rowIndex = idx;

    for(int c = 0; c < static_cast<int>(content.size()); ++c)
        content[c]->setIndex(rowIndex, c);
}

void ChatLine::visibilityChanged(bool visible)
{
    if(isVisible != visible)
    {
        for(ChatLineContent* c : content)
            c->visibilityChanged(visible);
    }

    isVisible = visible;
}

int ChatLine::getRowIndex() const
{
    return rowIndex;
}

ChatLineContent *ChatLine::getContent(int col) const
{
    if(col < static_cast<int>(content.size()) && col >= 0)
        return content[col];

    return nullptr;
}

ChatLineContent *ChatLine::getContent(QPointF scenePos) const
{
    for(ChatLineContent* c: content)
    {
        if(c->sceneBoundingRect().contains(scenePos))
            return c;
    }

    return nullptr;
}

void ChatLine::removeFromScene()
{
    for(ChatLineContent* c : content)
    {
        if(c->scene())
            c->scene()->removeItem(c);
    }
}

void ChatLine::addToScene(QGraphicsScene *scene)
{
    if(!scene)
        return;

    for(ChatLineContent* c : content)
        scene->addItem(c);
}

void ChatLine::setVisible(bool visible)
{
    for(ChatLineContent* c : content)
        c->setVisible(visible);
}

void ChatLine::selectionCleared()
{
    for(ChatLineContent* c : content)
        c->selectionCleared();
}

void ChatLine::selectionCleared(int col)
{
    if(col < static_cast<int>(content.size()) && content[col])
        content[col]->selectionCleared();
}

int ChatLine::getColumnCount()
{
    return content.size();
}

void ChatLine::updateBBox()
{
    bbox.setHeight(0);
    bbox.setWidth(width);

    for(ChatLineContent* c : content)
        bbox.setHeight(qMax(c->sceneBoundingRect().height(), bbox.height()));
}

QRectF ChatLine::boundingSceneRect() const
{
    return bbox;
}

void ChatLine::addColumn(ChatLineContent* item, ColumnFormat fmt)
{
    if(!item)
        return;

    format.push_back(fmt);
    content.push_back(item);
}

void ChatLine::replaceContent(int col, ChatLineContent *lineContent)
{
    if(col >= 0 && col < static_cast<int>(content.size()) && lineContent)
    {
        QGraphicsScene* scene = content[col]->scene();
        delete content[col];

        content[col] = lineContent;
        lineContent->setIndex(rowIndex, col);

        if(scene)
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

    qreal fixedWidth = (content.size()-1) * CELL_SPACING;
    qreal varWidth = 0.0; // used for normalisation

    for(int i = 0; i < static_cast<int>(format.size()); ++i)
    {
        if(format[i].policy == ColumnFormat::FixedSize)
            fixedWidth += format[i].size;
        else
            varWidth += format[i].size;
    }

    if(varWidth == 0.0)
        varWidth = 1.0;

    qreal leftover = qMax(0.0, width - fixedWidth);

    qreal maxVOffset = 0.0;
    qreal xOffset = 0.0;

    for(int i = 0; i < static_cast<int>(content.size()); ++i)
    {
        maxVOffset = qMax(maxVOffset, content[i]->getAscent());

        // calculate the effective width of the current column
        qreal width;
        if(format[i].policy == ColumnFormat::FixedSize)
            width = format[i].size;
        else
            width = format[i].size / varWidth * leftover;

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
        content[i]->setPos(scenePos.x() + xOffset + xAlign, scenePos.y());

        xOffset += width + CELL_SPACING;
    }

    for(int i = 0; i < static_cast<int>(content.size()); ++i)
    {
        // calculate vertical alignment
        // vertical alignment may depend on width, so we do it in a second pass
        qreal yOffset = 0.0;

        yOffset = maxVOffset - content[i]->getAscent();

        // reposition
        content[i]->setPos(content[i]->pos().x(), content[i]->pos().y() + yOffset);
    }

    updateBBox();
}

void ChatLine::moveBy(qreal deltaY)
{
    // reposition only
    for(ChatLineContent* c : content)
        c->moveBy(0, deltaY);

    bbox.moveTop(bbox.top() + deltaY);
}

bool ChatLine::lessThanBSRectTop(const ChatLine::Ptr lhs, const qreal rhs)
{
    return lhs->boundingSceneRect().top() < rhs;
}

bool ChatLine::lessThanBSRectBottom(const ChatLine::Ptr lhs, const qreal rhs)
{
    return lhs->boundingSceneRect().bottom() < rhs;
}

bool ChatLine::lessThanRowIndex(const ChatLine::Ptr lhs, const ChatLine::Ptr rhs)
{
    return lhs->getRowIndex() < rhs->getRowIndex();
}
