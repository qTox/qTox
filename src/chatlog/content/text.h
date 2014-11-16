#ifndef TEXT_H
#define TEXT_H

#include "../chatlinecontent.h"

#include <QTextDocument>
#include <QTextCursor>

class CustomTextDocument;

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

class Text : public ChatLineContent
{
public:
    Text(const QString& txt = "", QFont font = QFont(), bool enableElide = false);
    virtual ~Text();

    void setText(const QString& txt);

    virtual void setWidth(qreal width);

    virtual void selectionMouseMove(QPointF scenePos);
    virtual void selectionStarted(QPointF scenePos);
    virtual void selectionCleared();
    virtual void selectAll();
    virtual bool isOverSelection(QPointF scenePos) const;
    virtual QString getSelectedText() const;

    virtual QRectF boundingSceneRect() const;
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void visibilityChanged(bool isVisible);

    virtual qreal firstLineVOffset() const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    // dynamic resource management
    void ensureIntegrity();
    void freeResources();
    QSizeF idealSize();

    int cursorFromPos(QPointF scenePos) const;

private:
    CustomTextDocument* doc = nullptr;
    QString text;
    QString elidedText;
    QSizeF size;
    bool isVisible = false;
    bool elide = false;
    QTextCursor cursor;
    qreal vOffset = 0.0;
    qreal width = 0.0;
    QFont defFont;

};

#endif // TEXT_H
