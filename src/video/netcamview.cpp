/*
    Copyright © 2014-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "netcamview.h"
#include "camerasource.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/video/videosurface.h"
#include "src/widget/tool/movablewidget.h"
#include <QBoxLayout>
#include <QFrame>
#include <QLabel>

NetCamView::NetCamView(ToxPk friendPk, QWidget* parent)
    : GenericNetCamView(parent)
    , selfFrame{nullptr}
    , friendPk{friendPk}
    , e(false)
{
    videoSurface = new VideoSurface(Nexus::getProfile()->loadAvatar(friendPk), this);
    videoSurface->setMinimumHeight(256);

    verLayout->insertWidget(0, videoSurface, 1);

    selfVideoSurface = new VideoSurface(Nexus::getProfile()->loadAvatar(), this, true);
    selfVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    selfVideoSurface->setMouseTracking(true);
    selfVideoSurface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    selfFrame = new MovableWidget(videoSurface);
    selfFrame->show();

    QHBoxLayout* frameLayout = new QHBoxLayout(selfFrame);
    frameLayout->addWidget(selfVideoSurface);
    frameLayout->setMargin(0);

    updateRatio();
    connections +=
        connect(selfVideoSurface, &VideoSurface::ratioChanged, this, &NetCamView::updateRatio);

    connections += connect(videoSurface, &VideoSurface::boundaryChanged, [this]() {
        QRect boundingRect = videoSurface->getBoundingRect();
        updateFrameSize(boundingRect.size());
        selfFrame->setBoundary(boundingRect);
    });

    connections += connect(videoSurface, &VideoSurface::ratioChanged, [this]() {
        selfFrame->setMinimumWidth(selfFrame->minimumHeight() * selfVideoSurface->getRatio());
        QRect boundingRect = videoSurface->getBoundingRect();
        updateFrameSize(boundingRect.size());
        selfFrame->resetBoundary(boundingRect);
    });

    connections += connect(Nexus::getProfile(), &Profile::selfAvatarChanged,
                           [this](const QPixmap& pixmap) { selfVideoSurface->setAvatar(pixmap); });

    connections += connect(Nexus::getProfile(), &Profile::friendAvatarChanged,
                           [this](ToxPk friendPk, const QPixmap& pixmap) {
                               if (this->friendPk == friendPk)
                                   videoSurface->setAvatar(pixmap);
                           });

    QRect videoSize = Settings::getInstance().getCamVideoRes();
    qDebug() << "SIZER" << videoSize;
}

NetCamView::~NetCamView()
{
    for (QMetaObject::Connection conn : connections)
        disconnect(conn);
}

void NetCamView::show(VideoSource* source, const QString& title)
{
    setSource(source);
    selfVideoSurface->setSource(&CameraSource::getInstance());

    setTitle(title);
    QWidget::show();
}

void NetCamView::hide()
{
    setSource(nullptr);
    selfVideoSurface->setSource(nullptr);

    if (selfFrame)
        selfFrame->deleteLater();

    selfFrame = nullptr;

    QWidget::hide();
}

void NetCamView::setSource(VideoSource* s)
{
    videoSurface->setSource(s);
}

void NetCamView::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void NetCamView::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    selfFrame->resetBoundary(videoSurface->getBoundingRect());
}

void NetCamView::updateRatio()
{
    selfFrame->setMinimumWidth(selfFrame->minimumHeight() * selfVideoSurface->getRatio());
    selfFrame->setRatio(selfVideoSurface->getRatio());
}

void NetCamView::updateFrameSize(QSize size)
{
    selfFrame->setMaximumSize(size.height() / 3, size.width() / 3);

    if (selfFrame->maximumWidth() > selfFrame->maximumHeight())
        selfFrame->setMaximumWidth(selfFrame->maximumHeight() * selfVideoSurface->getRatio());
    else
        selfFrame->setMaximumHeight(selfFrame->maximumWidth() / selfVideoSurface->getRatio());
}

void NetCamView::toggleVideoPreview()
{
    if (selfFrame->isHidden()) {
        selfFrame->show();
    } else {
        selfFrame->hide();
    }
}
