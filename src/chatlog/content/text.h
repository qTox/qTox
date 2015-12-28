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

#ifndef TEXT_H
#define TEXT_H

#include "../chatlinecontent.h"

#include <QFont>

class QTextDocument;

class Text : public ChatLineContent
{
public:
    // txt: may contain html code
    // rawText: does not contain html code
    Text(const QString& txt = "", QFont font = QFont(), bool enableElide = false, const QString& rawText = QString(), const QColor c = Qt::black);
    virtual ~Text();

    void setText(const QString& txt);

    void setWidth(qreal width) final override;

    void selectionMouseMove(QPointF scenePos) final override;
    void selectionStarted(QPointF scenePos) final override;
    void selectionCleared() final override;
    void selectionDoubleClick(QPointF scenePos) final override;
    void selectionFocusChanged(bool focusIn) final override;
    bool isOverSelection(QPointF scenePos) const final override;
    QString getSelectedText() const final override;
    bool hasSelection() const final override;
    bool selectNext(const QString& search, Qt::CaseSensitivity sensitivity) final override;
    bool selectPrevious(const QString& search, Qt::CaseSensitivity sensitivity) final override;
    int setHighlight(const QString& highlight, Qt::CaseSensitivity sensitivity) final override;

    QRectF boundingRect() const final override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) final override;

    void visibilityChanged(bool keepInMemory) final override;

    qreal getAscent() const final override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) final override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) final override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) final override;

    QString getText() const final override;

protected:
    // dynamic resource management
    void regenerate();
    void freeResources();

    QSizeF idealSize();
    int cursorFromPos(QPointF scenePos, bool fuzzy = true) const;
    int getSelectionEnd() const;
    int getSelectionStart() const;
    QString extractSanitizedText(int from, int to) const;
    QString extractImgTooltip(int pos) const;

private:
    QTextDocument* doc = nullptr;
    QString text;
    QString rawText;
    QString elidedText;
    QString selectedText;
    QString highlightText;
    QSizeF size;
    bool keepInMemory = false;
    bool elide = false;
    bool dirty = false;
    bool selectionHasFocus = true;
    int selectionEnd = -1;
    int selectionAnchor = -1;
    qreal ascent = 0.0;
    qreal width = 0.0;
    QFont defFont;
    QColor color;
    Qt::CaseSensitivity sensitivity;
};

#endif // TEXT_H
