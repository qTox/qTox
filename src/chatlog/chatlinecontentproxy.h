#ifndef CHATLINECONTENTPROXY_H
#define CHATLINECONTENTPROXY_H

#include <QGraphicsProxyWidget>
#include "chatlinecontent.h"

class ChatLineContentProxy : public ChatLineContent
{
public:
    ChatLineContentProxy(QWidget* widget);

    virtual QRectF boundingRect() const;
    virtual QRectF boundingSceneRect() const;
    virtual void setWidth(qreal width);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QGraphicsProxyWidget* proxy;
};

#endif // CHATLINECONTENTPROXY_H
