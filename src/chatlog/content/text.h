/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#pragma once

#include "../chatlinecontent.h"
#include "src/widget/style.h"

#include <QFont>
#include <QTextCursor>

class QTextDocument;

class Text : public ChatLineContent
{
    Q_OBJECT

public:
    enum TextType
    {
        NORMAL,
        ACTION,
        CUSTOM
    };

    Text(const QString& txt = "", const QFont& font = QFont(), bool enableElide = false,
         const QString& rawText = QString(), const TextType& type = NORMAL, const QColor& custom = Style::getColor(Style::MainText));
    virtual ~Text();

    void setText(const QString& txt);
    void selectText(const QString& txt, const std::pair<int, int>& point);
    void selectText(const QRegularExpression& exp, const std::pair<int, int>& point);
    void deselectText();

    void setWidth(qreal width) final;

    void selectionMouseMove(QPointF scenePos) final;
    void selectionStarted(QPointF scenePos) final;
    void selectionCleared() final;
    void selectionDoubleClick(QPointF scenePos) final;
    void selectionTripleClick(QPointF scenePos) final;
    void selectionFocusChanged(bool focusIn) final;
    bool isOverSelection(QPointF scenePos) const final;
    QString getSelectedText() const final;
    void fontChanged(const QFont& font) final;

    QRectF boundingRect() const final;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) final;

    void visibilityChanged(bool keepInMemory) final;
    void reloadTheme() final;

    qreal getAscent() const final;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) final;

    QString getText() const final;
    QString getLinkAt(QPointF scenePos) const;

protected:
    // dynamic resource management
    void regenerate();
    void freeResources();

    virtual QSizeF idealSize();
    int cursorFromPos(QPointF scenePos, bool fuzzy = true) const;
    int getSelectionEnd() const;
    int getSelectionStart() const;
    bool hasSelection() const;
    QString extractSanitizedText(int from, int to) const;
    QString extractImgTooltip(int pos) const;

    QTextDocument* doc = nullptr;
    QSizeF size;
    qreal width = 0.0;

private:
    void selectText(QTextCursor& cursor, const std::pair<int, int>& point);
    QColor textColor() const;

    QString text;
    QString rawText;
    QString selectedText;
    bool keepInMemory = false;
    bool elide = false;
    bool dirty = false;
    bool selectionHasFocus = true;
    int selectionEnd = -1;
    int selectionAnchor = -1;
    qreal ascent = 0.0;
    QFont defFont;
    QString defStyleSheet;
    TextType textType;
    QColor color;
    QColor customColor;

    QTextCursor selectCursor;
    std::pair<int, int> selectPoint{0, 0};
};
