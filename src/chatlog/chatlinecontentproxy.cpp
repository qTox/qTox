#include "chatlinecontentproxy.h"
#include <QWidget>
#include <QDebug>

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget)
{
    proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
}

QRectF ChatLineContentProxy::boundingRect() const
{
    return proxy->boundingRect();
}

QRectF ChatLineContentProxy::boundingSceneRect() const
{
    return proxy->boundingRect().translated(scenePos());
}

void ChatLineContentProxy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    proxy->paint(painter, option, widget);
}

void ChatLineContentProxy::setWidth(qreal width)
{
    proxy->widget()->setFixedWidth(qMax(width,0.0));
}
