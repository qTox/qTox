/*
    Copyright © 2014-2018 by The qTox Project Contributors

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
#include "src/widget/style.h"

#include <QFont>

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

    virtual void setWidth(qreal width) final;

    virtual void selectionMouseMove(QPointF scenePos) final;
    virtual void selectionStarted(QPointF scenePos) final;
    virtual void selectionCleared() final;
    virtual void selectionDoubleClick(QPointF scenePos) final;
    virtual void selectionTripleClick(QPointF scenePos) final;
    virtual void selectionFocusChanged(bool focusIn) final;
    virtual bool isOverSelection(QPointF scenePos) const final;
    virtual QString getSelectedText() const final;
    virtual void fontChanged(const QFont& font) final;

    virtual QRectF boundingRect() const final;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) final;

    virtual void visibilityChanged(bool keepInMemory) final;
    virtual void reloadTheme() final override;

    virtual qreal getAscent() const final;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) final override;    

    virtual QString getText() const final;
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
};

#endif // TEXT_H
