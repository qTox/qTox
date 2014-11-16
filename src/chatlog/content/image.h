#ifndef IMAGE_H
#define IMAGE_H

#include "../chatlinecontent.h"

class Image : public QObject, public ChatLineContent
{
public:
    Image(QSizeF size, const QString &filename);

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void setWidth(qreal width);
    virtual QRectF boundingSceneRect() const;
    virtual qreal firstLineVOffset() const;

private:
    QSizeF size;
    QPixmap pmap;

};

#endif // IMAGE_H
