/*
    Copyright © 2014 by The qTox Project

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
#include "src/video/videosurface.h"
#include "src/widget/tool/movablewidget.h"
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>

NetCamView::NetCamView(QWidget* parent)
    : QWidget(parent)
    , mainLayout(new QHBoxLayout())
    , selfFrame{nullptr}
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setWindowTitle(tr("Tox video"));
    setMinimumSize(320,240);

    videoSurface = new VideoSurface(this);

    //mainLayout->addStretch();
    mainLayout->addWidget(videoSurface);
    //mainLayout->addStretch();

    selfVideoSurface = new VideoSurface(this);
    selfVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    selfVideoSurface->setMinimumSize(QSize(160, 120));
    selfVideoSurface->setSource(&CameraSource::getInstance());

    connect(&CameraSource::getInstance(), &CameraSource::deviceOpened, [this]()
    {
        connect(selfVideoSurface, &VideoSurface::drewNewFrame, this, &NetCamView::updateSize);
    });

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    button = new QPushButton();
    buttonLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &NetCamView::showMessageClicked);

    layout->addLayout(mainLayout);
    layout->addLayout(buttonLayout);

    setShowMessages(false);
}

void NetCamView::show(VideoSource *source, const QString &title)
{
    setSource(source);
    setTitle(title);

    QWidget::show();
    updateSize();
}

void NetCamView::hide()
{
    setSource(nullptr);

    if (selfFrame)
        selfFrame->deleteLater();

    selfFrame = nullptr;

    QWidget::hide();
}

void NetCamView::setSource(VideoSource *s)
{
    videoSurface->setSource(s);
}

void NetCamView::setTitle(const QString &title)
{
    setWindowTitle(title);
}

void NetCamView::setShowMessages(bool show, bool notify)
{
    if (show)
    {
        button->setText(tr("Show Messages"));

        if (notify)
            button->setIcon(QIcon("://ui/chatArea/info.svg"));
    }
    else
    {
        button->setText(tr("Hide Messages"));
        button->setIcon(QIcon());
    }
}

void NetCamView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateSize();
}

void NetCamView::updateSize()
{
    // Check there is room for a second video.
    // If so, then we will show the user video there too.
    //qDebug() << selfVideoSurface->getRect().size() == ;
    bool hasRoom = selfVideoSurface->getRect().width() != 0 && videoSurface->getRect().width() * 2 < layout()->contentsRect().width() - layout()->margin();

    if (mainLayout->indexOf(selfVideoSurface) != -1)
    {

        if (!hasRoom)
        {
            selfFrame = new MovableWidget(videoSurface);
            selfFrame->show();

            QHBoxLayout* camLayout = new QHBoxLayout(selfFrame);
            camLayout->addWidget(selfVideoSurface);
            camLayout->setMargin(0);

            //selfFrame->setBoundary(videoSurface->getRect());
            updateFrameSize();
        }
    }
    else
    {
        if (hasRoom)
        {
            if (selfFrame)
                selfFrame->deleteLater();

            selfFrame = nullptr;

            mainLayout->addWidget(selfVideoSurface);
        }
        else if (selfFrame)
        {
            updateFrameSize();
        }
    }

    disconnect(selfVideoSurface, &VideoSurface::drewNewFrame, this, &NetCamView::updateSize);
}

void NetCamView::updateFrameSize()
{
    QSize frameSize = selfVideoSurface->getFrameSize();
    float ratio = frameSize.width() / static_cast<float>(frameSize.height());
    QRect videoRect = videoSurface->getRect();
    int frameHeight = videoRect.height() / 3.0f;
    //selfFrame->resize(frameHeight * ratio, frameHeight);
    selfFrame->setBoundary(videoRect, QSize(frameHeight * ratio, frameHeight));
}
