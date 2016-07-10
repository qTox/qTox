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

#include "text.h"
#include "../documentcache.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QDebug>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QDesktopServices>
#include <QTextFragment>

#include "src/widget/style.h"

Text::Text(const QString& txt, const QFont& font, bool enableElide, const QString &rwText, const QColor c)
    : rawText(rwText)
    , elide(enableElide)
    , defFont(font)
    , defStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatArea/innerStyle.css"), font))
    , color(c)
{
    setText(txt);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

Text::~Text()
{
    if (doc)
        DocumentCache::getInstance().push(doc);
}

void Text::setText(const QString& txt)
{
    text = txt;
    dirty = true;
}

void Text::setWidth(qreal w)
{
    width = w;
    dirty = true;

    if (elide)
    {
        QFontMetrics metrics = QFontMetrics(defFont);
        elidedText = metrics.elidedText(text, Qt::ElideRight, width);
    }

    regenerate();
}

void Text::selectionMouseMove(QPointF scenePos)
{
    if (!doc)
        return;

    int cur = cursorFromPos(scenePos);
    if (cur >= 0)
    {
        selectionEnd = cur;
        selectedText = extractSanitizedText(getSelectionStart(), getSelectionEnd());
    }

    update();
}

void Text::selectionStarted(QPointF scenePos)
{
    int cur = cursorFromPos(scenePos);
    if (cur >= 0)
    {
        selectionEnd = cur;
        selectionAnchor = cur;
    }
}

void Text::selectionCleared()
{
    selectedText.clear();
    selectedText.squeeze();

    // Do not reset selectionAnchor!
    selectionEnd = -1;

    update();
}

void Text::selectionDoubleClick(QPointF scenePos)
{
    if (!doc)
        return;

    int cur = cursorFromPos(scenePos);

    if (cur >= 0)
    {
        QTextCursor cursor(doc);
        cursor.setPosition(cur);
        cursor.select(QTextCursor::WordUnderCursor);

        selectionAnchor = cursor.selectionStart();
        selectionEnd = cursor.selectionEnd();

        selectedText = extractSanitizedText(getSelectionStart(), getSelectionEnd());
    }

    update();
}

void Text::selectionFocusChanged(bool focusIn)
{
    selectionHasFocus = focusIn;
    update();
}

bool Text::isOverSelection(QPointF scenePos) const
{
    int cur = cursorFromPos(scenePos);
    if (getSelectionStart() < cur && getSelectionEnd() >= cur)
        return true;

    return false;
}

QString Text::getSelectedText() const
{
    return selectedText;
}

QRectF Text::boundingRect() const
{
    return QRectF(QPointF(0, 0), size);
}

void Text::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (doc)
    {
        painter->setClipRect(boundingRect());

        // draw selection
        QAbstractTextDocumentLayout::PaintContext ctx;
        QAbstractTextDocumentLayout::Selection sel;

        if (hasSelection())
        {
            sel.cursor = QTextCursor(doc);
            sel.cursor.setPosition(getSelectionStart());
            sel.cursor.setPosition(getSelectionEnd(), QTextCursor::KeepAnchor);
        }

        const QColor selectionColor = QColor::fromRgbF(0.23, 0.68, 0.91);
        sel.format.setBackground(selectionColor.lighter(selectionHasFocus ? 100 : 160));
        sel.format.setForeground(selectionHasFocus ? Qt::white : Qt::black);
        ctx.selections.append(sel);
        ctx.palette.setColor(QPalette::Text, color);

        // draw text
        doc->documentLayout()->draw(painter, ctx);
    }

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Text::visibilityChanged(bool visible)
{
    keepInMemory = visible;

    regenerate();
    update();
}

qreal Text::getAscent() const
{
    return ascent;
}

void Text::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        event->accept(); // grabber
}

void Text::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!doc)
        return;

    QString anchor = doc->documentLayout()->anchorAt(event->pos());

    // open anchor in browser
    if (!anchor.isEmpty())
        QDesktopServices::openUrl(anchor);
}

void Text::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!doc)
        return;

    QString anchor = doc->documentLayout()->anchorAt(event->pos());

    if (anchor.isEmpty())
        setCursor(Qt::IBeamCursor);
    else
        setCursor(Qt::PointingHandCursor);

    // tooltip
    setToolTip(extractImgTooltip(cursorFromPos(event->scenePos(), false)));
}

QString Text::getText() const
{
    return rawText;
}

void Text::regenerate()
{
    if (!doc)
    {
        doc = DocumentCache::getInstance().pop();
        dirty = true;
    }

    if (dirty)
    {
        doc->setDefaultFont(defFont);

        if (!elide)
        {
            doc->setDefaultStyleSheet(defStyleSheet);
            doc->setHtml(text);
        }
        else
        {
            doc->setPlainText(elidedText);
        }

        // wrap mode
        QTextOption opt;
        opt.setWrapMode(elide ? QTextOption::NoWrap : QTextOption::WrapAtWordBoundaryOrAnywhere);
        doc->setDefaultTextOption(opt);

        // width
        doc->setTextWidth(width);
        doc->documentLayout()->update();

        // update ascent
        if (doc->firstBlock().layout()->lineCount() > 0)
            ascent = doc->firstBlock().layout()->lineAt(0).ascent();

        // let the scene know about our change in size
        if (size != idealSize())
            prepareGeometryChange();

        // get the new width and height
        size = idealSize();

        dirty = false;
    }

    // if we are not visible -> free mem
    if (!keepInMemory)
        freeResources();
}

void Text::freeResources()
{
    DocumentCache::getInstance().push(doc);
    doc = nullptr;
}

QSizeF Text::idealSize()
{
    if (doc)
        return QSizeF(qMin(doc->idealWidth(), width), doc->size().height());

    return size;
}

int Text::cursorFromPos(QPointF scenePos, bool fuzzy) const
{
    if (doc)
        return doc->documentLayout()->hitTest(mapFromScene(scenePos), fuzzy ? Qt::FuzzyHit : Qt::ExactHit);

    return -1;
}

int Text::getSelectionEnd() const
{
    return qMax(selectionAnchor, selectionEnd);
}

int Text::getSelectionStart() const
{
    return qMin(selectionAnchor, selectionEnd);
}

bool Text::hasSelection() const
{
    return selectionEnd >= 0;
}

QString Text::extractSanitizedText(int from, int to) const
{
    if (!doc)
        return "";

    QString txt;
    QTextBlock block = doc->firstBlock();

    for (QTextBlock::Iterator itr = block.begin(); itr!=block.end(); ++itr)
    {
        int pos = itr.fragment().position(); //fragment position -> position of the first character in the fragment

        if (itr.fragment().charFormat().isImageFormat())
        {
            QTextImageFormat imgFmt = itr.fragment().charFormat().toImageFormat();
            QString key = imgFmt.name(); //img key (eg. key::D for :D)
            QString rune = key.mid(4);

            if (pos >= from && pos < to)
            {
                txt += rune;
                pos++;
            }
        }
        else
        {
            for (QChar c : itr.fragment().text())
            {
                if (pos >= from && pos < to)
                    txt += c;

                pos++;
            }
        }
    }

    return txt;
}

QString Text::extractImgTooltip(int pos) const
{
    for (QTextBlock::Iterator itr = doc->firstBlock().begin(); itr!=doc->firstBlock().end(); ++itr)
    {
        if (itr.fragment().contains(pos) && itr.fragment().charFormat().isImageFormat())
        {
            QTextImageFormat imgFmt = itr.fragment().charFormat().toImageFormat();
            return imgFmt.toolTip();
        }
    }

    return QString();
}
