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

#include "text.h"

#include "../customtextdocument.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QDebug>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QRegExp>
#include <QDesktopServices>

Text::Text(const QString& txt, QFont font, bool enableElide)
    : ChatLineContent()
    , elide(enableElide)
    , defFont(font)
{
    setText(txt);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

    ensureIntegrity();
    freeResources();
    //setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

Text::~Text()
{
    delete doc;
}

void Text::setText(const QString& txt)
{
    text = txt;
    dirty = true;

    ensureIntegrity();
    freeResources();
}

void Text::setWidth(qreal w)
{
    if(w == width)
        return;

    width = w;

    if(elide)
    {
        QFontMetrics metrics = QFontMetrics(defFont);
        elidedText = metrics.elidedText(text, Qt::ElideRight, width);
        dirty = true;
    }

    size = idealSize();

    ensureIntegrity();
    freeResources();
}

void Text::selectionMouseMove(QPointF scenePos)
{
    ensureIntegrity();
    int cur = cursorFromPos(scenePos);
    if(cur >= 0)
    {
        cursor.setPosition(cur, QTextCursor::KeepAnchor);
        selectedText = cursor.selectedText();
    }

    update();
}

void Text::selectionStarted(QPointF scenePos)
{
    ensureIntegrity();
    int cur = cursorFromPos(scenePos);
    if(cur >= 0)
        cursor.setPosition(cur);

    selectedText.clear();
    selectedText.squeeze();
}

void Text::selectionCleared()
{
    ensureIntegrity();
    cursor.setPosition(0);
    selectedText.clear();
    selectedText.squeeze();
    freeResources();

    update();
}

void Text::selectAll()
{
    ensureIntegrity();
    cursor.select(QTextCursor::Document);
    selectedText = text;
    update();
}

bool Text::isOverSelection(QPointF scenePos) const
{
    int cur = cursorFromPos(scenePos);
    if(cur >= 0 && cursor.selectionStart() < cur && cursor.selectionEnd() >= cur)
        return true;

    return false;
}

QString Text::getSelectedText() const
{
    return selectedText;
}

QRectF Text::boundingSceneRect() const
{
    return QRectF(scenePos(), size);
}

QRectF Text::boundingRect() const
{
    return QRectF(QPointF(0, 0), size);
}

void Text::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if(doc)
    {
        // draw selection
        QAbstractTextDocumentLayout::PaintContext ctx;
        QAbstractTextDocumentLayout::Selection sel;
        sel.cursor = cursor;
        sel.format.setBackground(QApplication::palette().color(QPalette::Highlight));
        sel.format.setForeground(QApplication::palette().color(QPalette::HighlightedText));
        ctx.selections.append(sel);

        // draw text
        doc->documentLayout()->draw(painter, ctx);
    }

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Text::visibilityChanged(bool visible)
{
    isVisible = visible;

    if(visible)
        ensureIntegrity();
    else
        freeResources();

    update();
}

qreal Text::getAscent() const
{
    return vOffset;
}

void Text::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        event->accept(); // grabber
}

void Text::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QString anchor = doc->documentLayout()->anchorAt(event->pos());

    // open anchors in browser
    if(!anchor.isEmpty())
        QDesktopServices::openUrl(anchor);
}

QString Text::getText() const
{
    return text;
}

void Text::ensureIntegrity()
{
    if(!doc || dirty)
    {
        doc = new CustomTextDocument();
        doc->setDefaultFont(defFont);

        if(!elide)
        {
            doc->setHtml(text);
        }
        else
        {
            QTextOption opt;
            opt.setWrapMode(QTextOption::NoWrap);
            doc->setDefaultTextOption(opt);
            doc->setPlainText(elidedText);
        }

        cursor = QTextCursor(doc);
        dirty = false;
    }

    doc->setTextWidth(width);
    doc->documentLayout()->update();

    if(doc->firstBlock().layout()->lineCount() > 0)
        vOffset = doc->firstBlock().layout()->lineAt(0).ascent();

    if(size != idealSize())
        prepareGeometryChange();

    size = idealSize();
}

void Text::freeResources()
{
    if(doc && !isVisible && !cursor.hasSelection())
    {
        delete doc;
        doc = nullptr;
        cursor = QTextCursor();
    }
}

QSizeF Text::idealSize()
{
    if(doc)
        return QSizeF(qMin(doc->idealWidth(), width), doc->size().height());

    return size;
}

int Text::cursorFromPos(QPointF scenePos) const
{
    if(doc)
        return doc->documentLayout()->hitTest(mapFromScene(scenePos), Qt::FuzzyHit);

    return -1;
}
