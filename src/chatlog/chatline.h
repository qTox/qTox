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

#ifndef CHATLINE_H
#define CHATLINE_H

#include <QTextLayout>

class ChatLog;
class ChatLineContent;
class QGraphicsScene;
class QStyleOptionGraphicsItem;

struct ColumnFormat
{
    enum Policy {
        FixedSize,
        VariableSize,
    };

    enum Align {
        Left,
        Center,
        Right,
    };

    ColumnFormat() {}
    ColumnFormat(qreal s, Policy p, int valign = -1, Align halign = Left)
        : size(s)
        , policy(p)
        , vAlignCol(valign)
        , hAlign(halign)
    {}

    qreal size = 1.0;
    Policy policy = VariableSize;
    int vAlignCol = -1;
    Align hAlign = Left;
};

using ColumnFormats = QVector<ColumnFormat>;

class ChatLine
{
public:
    explicit ChatLine(QGraphicsScene* scene);
    virtual ~ChatLine();

    virtual QRectF boundingSceneRect() const;

    void addColumn(ChatLineContent* item, ColumnFormat fmt);
    void replaceContent(int col, ChatLineContent* lineContent);

    void layout(qreal width, QPointF scenePos);
    void layout(QPointF scenePos);

    void selectionCleared();
    void selectionCleared(int col);
    void selectAll();
    void selectAll(int col);

    int getColumnCount();
    int getRowIndex() const;

    bool isOverSelection(QPointF scenePos);

private:
    QPointF mapToContent(ChatLineContent* c, QPointF pos);
    void updateBBox();

    friend class ChatLog;
    void setRowIndex(int idx);
    void visibilityChanged(bool visible);

private:
    int rowIndex = -1;
    QGraphicsScene* scene = nullptr;
    QVector<ChatLineContent*> content; // 3 columns
    QVector<ColumnFormat> format;
    qreal width;
    QRectF bbox;
    QPointF pos;
    bool isVisible = false;

};

#endif // CHATLINE_H
