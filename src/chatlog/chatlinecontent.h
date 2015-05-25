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

#ifndef CHATLINECONTENT_H
#define CHATLINECONTENT_H

#include <QGraphicsItem>

class ChatLine;

class ChatLineContent : public QGraphicsItem
{
public:
    enum GraphicsItemType
    {
        ChatLineContentType = QGraphicsItem::UserType + 1,
    };

    int getColumn() const;
    int getRow() const;

    virtual void setWidth(qreal width) = 0;
    virtual int type() const final;

    virtual void selectionMouseMove(QPointF scenePos);
    virtual void selectionStarted(QPointF scenePos);
    virtual void selectionCleared();
    virtual void selectionDoubleClick(QPointF scenePos);
    virtual void selectionFocusChanged(bool focusIn);
    virtual bool isOverSelection(QPointF scenePos) const;
    virtual QString getSelectedText() const;

    virtual QString getText() const;

    virtual qreal getAscent() const;

    virtual QRectF boundingRect() const = 0;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

    virtual void visibilityChanged(bool visible);

private:
    friend class ChatLine;
    void setIndex(int row, int col);

private:
    int row = -1;
    int col = -1;
};

#endif // CHATLINECONTENT_H
