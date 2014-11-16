#include "chatlinecontent.h"

void ChatLineContent::setChatLine(ChatLine* chatline)
{
    line = chatline;
}

ChatLine* ChatLineContent::getChatLine() const
{
    return line;
}

void ChatLineContent::setIndex(int r, int c)
{
    row = r;
    col = c;
}

int ChatLineContent::getColumn() const
{
    return col;
}

int ChatLineContent::getRow() const
{
    return row;
}

int ChatLineContent::type() const
{
    return GraphicsItemType::ChatLineContentType;
}

void ChatLineContent::selectionMouseMove(QPointF)
{

}

void ChatLineContent::selectionStarted(QPointF)
{

}

void ChatLineContent::selectionCleared()
{
}

void ChatLineContent::selectAll()
{
}

bool ChatLineContent::isOverSelection(QPointF) const
{
    return false;
}

QString ChatLineContent::getSelectedText() const
{
    return QString();
}

qreal ChatLineContent::firstLineVOffset() const
{
    return 0.0;
}

void ChatLineContent::visibilityChanged(bool)
{

}
