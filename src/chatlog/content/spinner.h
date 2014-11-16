#ifndef SPINNER_H
#define SPINNER_H

#include "../chatlinecontent.h"

#include <QTimer>
#include <QObject>

class Spinner : public QObject, public ChatLineContent
{
    Q_OBJECT
public:
    Spinner(QSizeF size);

    virtual QRectF boundingRect() const;
    virtual QRectF boundingSceneRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void setWidth(qreal width);
    virtual void visibilityChanged(bool visible);
    virtual qreal firstLineVOffset() const;

private slots:
    void timeout();

private:
    QSizeF size;
    QPixmap pmap;
    qreal rot;
    QTimer timer;

};

#endif // SPINNER_H
