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

#include <QMouseEvent>
#include <memory>
#include <vector>

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
    ColumnFormat(qreal s, Policy p, Align halign = Left)
        : size(s)
        , policy(p)
        , hAlign(halign)
    {}

    qreal size = 1.0;
    Policy policy = VariableSize;
    Align hAlign = Left;
};

using ColumnFormats = QVector<ColumnFormat>;

class ChatLine
{
public:
    using Ptr = std::shared_ptr<ChatLine>;

    explicit ChatLine();
    virtual ~ChatLine();

    virtual QRectF boundingSceneRect() const;

    void replaceContent(int col, ChatLineContent* lineContent);

    void layout(qreal width, QPointF scenePos);
    void moveBy(qreal deltaY);

    void selectionCleared();
    void selectionCleared(int col);
    void selectAll();
    void selectAll(int col);

    int getColumnCount();
    int getRowIndex() const;
    ChatLineContent* getContent(int col) const;
    ChatLineContent* getContent(QPointF scenePos) const;

    bool isOverSelection(QPointF scenePos);

    void removeFromScene();
    void addToScene(QGraphicsScene* scene);

    void setVisible(bool visible);

protected:
    QPointF mapToContent(ChatLineContent* c, QPointF pos);
    void addColumn(ChatLineContent* item, ColumnFormat fmt);
    void updateBBox();

    friend class ChatLog;
    void setRowIndex(int idx);
    void visibilityChanged(bool visible);

private:
    int rowIndex = -1;
    std::vector<ChatLineContent*> content; // 3 columns
    std::vector<ColumnFormat> format;
    qreal width;
    QRectF bbox;
    bool isVisible = false;

};

#endif // CHATLINE_H
