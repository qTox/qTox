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
#include <QFrame>

NetCamView::NetCamView(int friendId, QWidget* parent)
    : QWidget(parent)
    , mainLayout(new QHBoxLayout())
    , selfFrame{nullptr}
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setWindowTitle(tr("Tox video"));

    videoSurface = new VideoSurface(friendId, this);
    videoSurface->setStyleSheet("background-color: blue;");
    videoSurface->setMinimumHeight(256);
    videoSurface->setContentsMargins(6, 6, 6, 6);

    mainLayout->addWidget(videoSurface, 1);

    selfFrame = new MovableWidget(videoSurface);
    selfFrame->show();

    selfVideoSurface = new VideoSurface(-1, selfFrame);
    selfVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    selfVideoSurface->setMouseTracking(true);
    selfVideoSurface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout* frameLayout = new QHBoxLayout(selfFrame);
    frameLayout->addWidget(selfVideoSurface);
    frameLayout->setMargin(0);

    connect(&CameraSource::getInstance(), &CameraSource::deviceOpened, [this]()
    {
        //qDebug() << "Device changed";
        //selfFrame->setRatio(selfVideoSurface->getRatio());
        //selfFrame->resetBoundary();
        //connect(selfVideoSurface, &VideoSurface::drewNewFrame, this, &NetCamView::updateSize);
    });

    connect(selfVideoSurface, &VideoSurface::ratioChanged, [this]()
    {
        qDebug() << "Ratio changed";
        selfFrame->setRatio(selfVideoSurface->getRatio());
    });

    connect(videoSurface, &VideoSurface::boundaryChanged, [this]()
    {
        selfFrame->setBoundary(videoSurface->getBoundingRect());
    });

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    button = new QPushButton();
    buttonLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &NetCamView::showMessageClicked);

    layout->addLayout(mainLayout, 1);
    layout->addLayout(buttonLayout);

    QFrame* lineFrame = new QFrame(this);
    lineFrame->setStyleSheet("border: 1px solid #c1c1c1;");
    lineFrame->setFrameShape(QFrame::HLine);
    lineFrame->setMaximumHeight(1);
    layout->addWidget(lineFrame);

    setShowMessages(false);

    setStyleSheet("NetCamView { background-color: #c1c1c1; }");
}

void NetCamView::show(VideoSource *source, const QString &title)
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
    //updateSize();
}

void NetCamView::showEvent(QShowEvent *event)
{
    selfFrame->resetBoundary(videoSurface->getBoundingRect());
}

void NetCamView::updateSize()
{
    qDebug() << selfFrame->geometry();
    /*qDebug() << videoSurface->size();
    QSize usableSize = mainLayout->contentsRect().size();
    int possibleWidth = usableSize.height() * videoSurface->getRatio();

    QSize initial = videoSurface->sizeHint();

    if (!initial.isValid())
        return;

    if (possibleWidth > usableSize.width())
        videoSurface->setSizeHint(usableSize.width(), usableSize.width() / videoSurface->getRatio());
   else
       videoSurface->setSizeHint(usableSize.height() * videoSurface->getRatio(), usableSize.height());

    videoSurface->updateGeometry();

    QSize newSize = videoSurface->sizeHint();
    QSizeF initialSize = initial;
    selfFrame->setRatio(selfVideoSurface->getRatio());*/

    //selfFrame->setRatio(selfVideoSurface->getRatio());
    //selfFrame->resetBoundary(videoSurface->getBoundingRect());
    //selfFrame->setBoundary(newSize, initial, newSize.width() / initialSize.width(), newSize.height() / initialSize.height());
}
