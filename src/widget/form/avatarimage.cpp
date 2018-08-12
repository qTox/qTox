#include "avatarimage.h"

AvatarImage::AvatarImage(int h, int w, QWidget * parent) : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(0, 0, w, h, this);
    setScene(m_scene);

    size.setHeight(h);
    size.setWidth(w);

    renderTarget = new QPixmap(size);

    resize(size);

    setSceneRect(0, 0, size.width(), size.height()); /* x, y, width, height */
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    QImage imageClose(":/ui/fileTransferInstance/no.svg");
    imageClose = imageClose.scaled(
                size.width() / 2,
                size.height() / 2,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    QImage imageOpen(":/ui/fileTransferInstance/browse.svg");
    imageOpen = imageOpen.scaled(
                size.width() / 2,
                size.height() / 2,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

    itemAvatar = new QGraphicsPixmapItem();
    setDefaultAvatar();
    itemClose = new GraphicsPixmapItem(QPixmap::fromImage(imageClose));
    itemOpen = new GraphicsPixmapItem(QPixmap::fromImage(imageOpen));

    m_scene->addItem(itemAvatar);
    m_scene->addItem(itemClose);
    m_scene->addItem(itemOpen);

    itemClose->setToolTip(tr("drop avatar"));
    itemOpen->setToolTip(tr("select avatar"));

    itemClose->setPos(
                size.width() / 2,
                0);
    itemOpen->setPos(
                0,
                size.height() / 2);

    itemClose->setOpacity(opacity);
    itemOpen->setOpacity(opacity);

    connect(itemClose, SIGNAL(clicked()), this, SLOT(mousePressDropButton()) );
    connect(itemOpen, SIGNAL(clicked()), this, SLOT(mousePressOpenButton()) );
}

void AvatarImage::setDefaultAvatar()
{
    QImage imgAvatar(":img/contact_dark.svg");
    setAvatar(imgAvatar);
}

void AvatarImage::mousePressOpenButton(void)
{
    emit openAvatar();
}

void AvatarImage::setAvatar(QImage imgAvatar)
{
    imgAvatar = imgAvatar.scaled(
                size.width() ,
                size.height() ,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    QPixmap pixmap = getRoundedAvatar(imgAvatar);

    itemAvatar->setPixmap(pixmap);
}

void AvatarImage::mousePressDropButton()
{
    setDefaultAvatar();
    emit dropAvatar();
}

QPixmap AvatarImage::getRoundedAvatar(QImage& img)
{
    delete renderTarget;
    renderTarget = new QPixmap(size);

    renderTarget->fill(Qt::transparent);

    const QPixmap pixmap = QPixmap::fromImage(img);

    QPixmap mask = QPixmap::fromImage(QImage(":/img/avatar_mask.svg"));
    mask = mask.scaled(size,Qt::KeepAspectRatio,Qt::SmoothTransformation);

    QPainter painter(renderTarget);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(0,0, pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, mask);
    painter.end();
    return *renderTarget;
}

