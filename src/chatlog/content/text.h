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

#ifndef TEXT_H
#define TEXT_H

#include "../chatlinecontent.h"

#include <QTextDocument>
#include <QTextCursor>

class CustomTextDocument;

class Text : public ChatLineContent
{
public:
    Text(const QString& txt = "", QFont font = QFont(), bool enableElide = false, const QString& rawText = QString());
    virtual ~Text();

    void setText(const QString& txt);

    virtual void setWidth(qreal width) override;

    virtual void selectionMouseMove(QPointF scenePos) override;
    virtual void selectionStarted(QPointF scenePos) override;
    virtual void selectionCleared() override;
    virtual void selectAll() override;
    virtual bool isOverSelection(QPointF scenePos) const override;
    virtual QString getSelectedText() const override;

    virtual QRectF boundingSceneRect() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    virtual void visibilityChanged(bool isVisible) override;

    virtual qreal getAscent() const override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual QString getText() const override;

protected:
    // dynamic resource management
    void regenerate();
    void freeResources();

    QSizeF idealSize();
    int cursorFromPos(QPointF scenePos) const;

private:
    CustomTextDocument* doc = nullptr;
    QString text;
    QString rawText;
    QString elidedText;
    QString selectedText;
    QSizeF size;
    bool isVisible = false;
    bool elide = false;
    bool dirty = false;
    QTextCursor cursor;
    qreal ascent = 0.0;
    qreal width = 0.0;
    QFont defFont;

};

#endif // TEXT_H
