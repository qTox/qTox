#ifndef AVATARIMAGE_H
#define AVATARIMAGE_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

#include "graphicspixmapitem.h"


class AvatarImage : public QGraphicsView
{
    Q_OBJECT
public:
    explicit AvatarImage(int h, int w, QWidget * parent = 0);

signals:
    void openAvatar(void);
    void dropAvatar(void);

public slots:
    void setAvatar(QImage imgAvatar);

private slots:
    void mousePressOpenButton(void);
    void mousePressDropButton(void);

private:

    void setDefaultAvatar();

    QPixmap getRoundedAvatar(QImage& img);

    QGraphicsScene *m_scene;
    QSize size;
    float opacity = 0.2;

    GraphicsPixmapItem *itemClose;
    GraphicsPixmapItem *itemOpen;
    QGraphicsPixmapItem *itemAvatar;

    QPixmap *renderTarget;

};

#endif // AVATARIMAGE_H
