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

ChatLine::ChatLine(QGraphicsScene* grScene)
    : scene(grScene)
{

}

ChatLine::~ChatLine()
{
    for(ChatLineContent* c : content)
    {
        scene->removeItem(c);
        delete c;
    }
}

void ChatLine::setRowIndex(int idx)
{
    rowIndex = idx;

    for(int c = 0; c < content.size(); ++c)
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
    if(col < content.size() && col >= 0)
        return content[col];

    return nullptr;
}

void ChatLine::selectionCleared()
{
    for(ChatLineContent* c : content)
        c->selectionCleared();
}

void ChatLine::selectionCleared(int col)
{
    if(col < content.size() && content[col])
        content[col]->selectionCleared();
}

void ChatLine::selectAll()
{
    for(ChatLineContent* c : content)
        c->selectAll();
}

void ChatLine::selectAll(int col)
{
    if(col < content.size() && content[col])
        content[col]->selectAll();
}

int ChatLine::getColumnCount()
{
    return content.size();
}

void ChatLine::updateBBox()
{
    bbox = QRectF();
    bbox.setTop(pos.y());
    bbox.setLeft(pos.x());
    bbox.setWidth(width);

    for(ChatLineContent* c : content)
        bbox.setHeight(qMax(c->sceneBoundingRect().bottom() - pos.y(), bbox.height()));
}

QRectF ChatLine::boundingSceneRect() const
{
    return bbox;
}

void ChatLine::addColumn(ChatLineContent* item, ColumnFormat fmt)
{
    if(!item)
        return;

    item->setChatLine(this);
    scene->addItem(item);

    format << fmt;
    content << item;
}

void ChatLine::replaceContent(int col, ChatLineContent *lineContent)
{
    if(col >= 0 && col < content.size() && lineContent)
    {
        scene->removeItem(content[col]);
        delete content[col];

        content[col] = lineContent;
        scene->addItem(content[col]);

        layout(width, pos);
        content[col]->visibilityChanged(isVisible);
        content[col]->update();
    }
}

void ChatLine::layout(qreal w, QPointF scenePos)
{
    width = w;
    pos = scenePos;

    qreal fixedWidth = 0.0;
    qreal varWidth = 0.0; // used for normalisation

    for(int i = 0; i < format.size(); ++i)
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

    for(int i = 0; i < content.size(); ++i)
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
        content[i]->setPos(pos.x() + xOffset + xAlign, pos.y());

        xOffset += width;
    }

    for(int i = 0; i < content.size(); ++i)
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

void ChatLine::layout(QPointF scenePos)
{
    // reposition only
    QPointF offset = pos - scenePos;
    for(ChatLineContent* c : content)
        c->setPos(c->pos() - offset);

    pos = scenePos;

    updateBBox();
}
