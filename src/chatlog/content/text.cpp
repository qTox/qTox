#include "text.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QDebug>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>
#include <QDesktopServices>

Text::Text(const QString& txt, bool enableElide)
    : ChatLineContent()
    , elide(enableElide)
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
    text.replace("\n", "<br/>");

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
        QFontMetrics metrics = QFontMetrics(QFont());
        elidedText = metrics.elidedText(text, Qt::ElideRight, width);
    }

    ensureIntegrity();
    freeResources();
}

void Text::selectionMouseMove(QPointF scenePos)
{
    ensureIntegrity();
    int cur = cursorFromPos(scenePos);
    if(cur >= 0)
        cursor.setPosition(cur, QTextCursor::KeepAnchor);

    update();
}

void Text::selectionStarted(QPointF scenePos)
{
    ensureIntegrity();
    int cur = cursorFromPos(scenePos);
    if(cur >= 0)
        cursor.setPosition(cur);
}

void Text::selectionCleared()
{
    ensureIntegrity();
    cursor.setPosition(0);
    freeResources();

    update();
}

void Text::selectAll()
{
    ensureIntegrity();
    cursor.select(QTextCursor::Document);
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
    return cursor.selectedText();
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
}

qreal Text::firstLineVOffset()
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

QString Text::toString() const
{
    return text;
}

void Text::ensureIntegrity()
{
    if(!doc)
    {
        doc = new QTextDocument();
        doc->setUndoRedoEnabled(false);
        doc->setUseDesignMetrics(true);

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
    }

    doc->setTextWidth(width);
    doc->documentLayout()->update();

    if(doc->firstBlock().layout()->lineCount() > 0)
        vOffset = doc->firstBlock().layout()->lineAt(0).height();

    if(size != idealSize())
    {
        prepareGeometryChange();
        size = idealSize();
    }
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
        return QSizeF(doc->idealWidth(), doc->size().height());

    return QSizeF();
}

int Text::cursorFromPos(QPointF scenePos) const
{
    if(doc)
        return doc->documentLayout()->hitTest(mapFromScene(scenePos), Qt::ExactHit);

    return -1;
}
