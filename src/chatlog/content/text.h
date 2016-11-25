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
    Text(const QString& txt = "", const QFont& font = QFont(), bool enableElide = false, const QString& rawText = QString(), const QColor c = Qt::black);
    virtual ~Text();

    void setText(const QString& txt);

    virtual void setWidth(qreal width) final;

    void selectionMouseMove(QPointF scenePos) final;
    void selectionStarted(QPointF scenePos) final;
    void selectionCleared() final;
    void selectionDoubleClick(QPointF scenePos) final;
    void selectionFocusChanged(bool focusIn) final;
    bool isOverSelection(QPointF scenePos) const final;
    QString getSelectedText() const final;

    QRectF boundingRect() const final;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) final;

    void visibilityChanged(bool keepInMemory) final;

    qreal getAscent() const final;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) final;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) final;

    QString getText() const final;

protected:
    // dynamic resource management
    void regenerate();
    void freeResources();

    QSizeF idealSize();
    int cursorFromPos(QPointF scenePos, bool fuzzy = true) const;
    int getSelectionEnd() const;
    int getSelectionStart() const;
    bool hasSelection() const;
    QString extractSanitizedText(int from, int to) const;
    QString extractImgTooltip(int pos) const;

private:
    QTextDocument* doc = nullptr;
    QString text;
    QString rawText;
    QString selectedText;
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
    QString defStyleSheet;
    QColor color;

};

#endif // TEXT_H
