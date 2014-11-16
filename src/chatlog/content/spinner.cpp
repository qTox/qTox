#include "spinner.h"

#include <QPainter>
#include <QDebug>

Spinner::Spinner(QSizeF Size)
    : size(Size)
{
    pmap.load(":/ui/chatArea/spinner.png");

    timer.setInterval(33); // 30Hz
    timer.setSingleShot(false);
    timer.start();

    QObject::connect(&timer, &QTimer::timeout, this, &Spinner::timeout);
}

QRectF Spinner::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

QRectF Spinner::boundingSceneRect() const
{
    return QRectF(scenePos(), size);
}

void Spinner::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QTransform rotMat;
    rotMat.translate(size.width() / 2.0, size.height() / 2.0);
    rotMat.rotate(rot);
    rotMat.translate(-size.width() / 2.0, -size.height() / 2.0);

    painter->translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->setTransform(rotMat, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, size.width(), size.height(), pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Spinner::setWidth(qreal width)
{
    Q_UNUSED(width)
}

void Spinner::visibilityChanged(bool visible)
{
    if(visible)
        timer.start();
    else
        timer.stop();
}

qreal Spinner::firstLineVOffset() const
{
    return size.height() / 4.0;
}

void Spinner::timeout()
{
    rot += 8;
    update();
}
