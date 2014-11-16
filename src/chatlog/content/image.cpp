#include "image.h"

#include <QPainter>

Image::Image(QSizeF Size, const QString& filename)
    : size(Size)
{
    pmap.load(filename);
}

QRectF Image::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

QRectF Image::boundingSceneRect() const
{
    return QRectF(scenePos(), size);
}

qreal Image::firstLineVOffset() const
{
    return size.height() / 4.0;
}

void Image::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, size.width(), size.height(), pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Image::setWidth(qreal width)
{
    Q_UNUSED(width)
}
