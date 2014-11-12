#include "chatline.h"
#include "chatlog.h"
#include "chatlinecontent.h"

#include <QTextLine>
#include <QDebug>
#include <QGraphicsScene>

ChatLine::ChatLine(QGraphicsScene* grScene)
    : scene(grScene)
{

}

ChatLine::~ChatLine()
{
    for(ChatLineContent* c : content)
    {
        scene->removeItem(c);
        delete c;
    }
}

void ChatLine::setRowIndex(int idx)
{
    rowIndex = idx;

    for(int c = 0; c < content.size(); ++c)
        content[c]->setIndex(rowIndex, c);
}

void ChatLine::visibilityChanged(bool visible)
{
    if(isVisible != visible)
    {
        for(ChatLineContent* c : content)
            c->visibilityChanged(visible);
    }

    isVisible = visible;
}

int ChatLine::getRowIndex() const
{
    return rowIndex;
}

void ChatLine::selectionCleared()
{
    for(ChatLineContent* c : content)
        c->selectionCleared();
}

void ChatLine::selectionCleared(int col)
{
    if(col < content.size() && content[col])
        content[col]->selectionCleared();
}

void ChatLine::selectAll()
{
    for(ChatLineContent* c : content)
        c->selectAll();
}

void ChatLine::selectAll(int col)
{
    if(col < content.size() && content[col])
        content[col]->selectAll();
}

int ChatLine::getColumnCount()
{
    return content.size();
}

void ChatLine::updateBBox()
{
    bbox = QRectF();
    bbox.setTop(pos.y());
    bbox.setLeft(pos.x());
    bbox.setWidth(width);

    for(ChatLineContent* c : content)
        bbox.setHeight(qMax(c->sceneBoundingRect().height(), bbox.height()));
}

QRectF ChatLine::boundingSceneRect() const
{
    return bbox;
}

void ChatLine::addColumn(ChatLineContent* item, ColumnFormat fmt)
{
    if(!item)
        return;

    item->setChatLine(this);
    scene->addItem(item);

    format << fmt;
    content << item;
}

void ChatLine::replaceContent(int col, ChatLineContent *lineContent)
{
    if(col >= 0 && col < content.size() && lineContent)
    {
        scene->removeItem(content[col]);
        delete content[col];

        content[col] = lineContent;
        scene->addItem(content[col]);
    }
}

void ChatLine::layout(qreal w, QPointF scenePos)
{
    width = w;
    pos = scenePos;

    qreal fixedWidth = 0.0;
    qreal varWidth = 0.0; // used for normalisation

    for(int i = 0; i < format.size(); ++i)
    {
        if(format[i].policy == ColumnFormat::FixedSize)
            fixedWidth += format[i].size;
        else
            varWidth += format[i].size;
    }

    if(varWidth == 0.0)
        varWidth = 1.0;

    qreal leftover = qMax(0.0, width - fixedWidth);

    qreal xOffset = 0.0;
    for(int i = 0; i < content.size(); ++i)
    {
        // calculate the effective width of the current column
        qreal width;
        if(format[i].policy == ColumnFormat::FixedSize)
            width = format[i].size;
        else
            width = format[i].size / varWidth * leftover;

        // set the width of the current column as
        // firstLineVOffset() may depend on the current width
        content[i]->setWidth(width);

        // calculate vertical alignment
        qreal yOffset = 0.0;
        if(format[i].vAlignCol >= 0 && format[i].vAlignCol < content.size())
            yOffset = content[format[i].vAlignCol]->firstLineVOffset() - content[i]->firstLineVOffset();

        // calculate horizontal alignment
        qreal xAlign = 0.0;
        switch(format[i].hAlign)
        {
            case ColumnFormat::Right:
                xAlign = width - content[i]->boundingRect().width();
                break;
            case ColumnFormat::Center:
                xAlign = (width - content[i]->boundingRect().width()) / 2.0;
                break;
            default:
                break;
        }

        // reposition
        content[i]->setPos(pos.x() + xOffset + xAlign, pos.y() + yOffset);

        xOffset += width;
    }

    updateBBox();
}

void ChatLine::layout(QPointF scenePos)
{
    // reposition only
    QPointF offset = pos - scenePos;
    for(ChatLineContent* c : content)
        c->setPos(c->pos() - offset);

    pos = scenePos;

    updateBBox();
}
