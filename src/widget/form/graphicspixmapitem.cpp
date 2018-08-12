#include "graphicspixmapitem.h"

void GraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawPixmap(m_imageRect.toRect(), m_pixmap.scaled(m_imageRect.toRect().width(),
                                                          m_imageRect.toRect().height()));
}

void GraphicsPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect;
    QPropertyAnimation *anim = new QPropertyAnimation( opacity, "opacity" );
    setGraphicsEffect(opacity);

    anim->setDuration( 500 );
    anim->setStartValue( 0.2 );
    anim->setEndValue( 1.0 );
    anim->setEasingCurve( QEasingCurve::InCubic );
    anim->start(QAbstractAnimation::KeepWhenStopped);
}

void GraphicsPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect;
    QPropertyAnimation *anim = new QPropertyAnimation( opacity, "opacity" );
    setGraphicsEffect(opacity);

    anim->setDuration( 500 );
    anim->setStartValue( 1.0 );
    anim->setEndValue( 0.2 );
    anim->setEasingCurve( QEasingCurve::InCubic );
    anim->start(QAbstractAnimation::KeepWhenStopped);
}

void GraphicsPixmapItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(Qt::PointingHandCursor);
    setOpacity(1.0);
    event->ignore();
}

GraphicsPixmapItem::GraphicsPixmapItem(const QPixmap & pixmap, QGraphicsItem * parent) :
    QGraphicsItem(parent),
    m_pixmap(pixmap)
{
    m_imageRect = QRectF(pixmap.rect());
    setFlags(ItemIsSelectable |
           ItemIsMovable |
           ItemSendsGeometryChanges |
           ItemIsFocusable);
    setAcceptHoverEvents(true);
}

void GraphicsPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    emit clicked();
}

void GraphicsPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

void GraphicsPixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    update();
}

QRectF GraphicsPixmapItem::boundingRect() const
{
    return m_imageRect;
}
