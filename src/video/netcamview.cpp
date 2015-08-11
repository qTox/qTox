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

    videoSurface = new VideoSurface(this);
    videoSurface->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    videoSurface->setStyleSheet("background-color: blue;");
    videoSurface->setMinimum(128);

    mainLayout->addStretch(1);
    //QWidget* wid = new QWidget(this);
    //wid->setMinimumSize(600, 600);
    QVBoxLayout* horLayout = new QVBoxLayout();
    horLayout->addStretch(1);
    horLayout->addWidget(videoSurface);
    horLayout->addStretch(1);
    mainLayout->addLayout(horLayout);

    selfFrame = new MovableWidget(videoSurface);
    selfFrame->show();

    selfVideoSurface = new VideoSurface(selfFrame);
    selfVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    selfVideoSurface->setMouseTracking(true);

    QHBoxLayout* frameLayout = new QHBoxLayout(selfFrame);
    frameLayout->addWidget(selfVideoSurface);
    frameLayout->setMargin(0);

    //mainLayout->addWidget(selfVideoSurface);
    mainLayout->addStretch(1);

    /*connect(&CameraSource::getInstance(), &CameraSource::deviceOpened, [this]()
    {
        connect(selfVideoSurface, &VideoSurface::drewNewFrame, this, &NetCamView::updateSize);
    });*/

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    button = new QPushButton();
    buttonLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &NetCamView::showMessageClicked);

    layout->addLayout(mainLayout, 1);
    layout->addLayout(buttonLayout);

    setShowMessages(false);

    setStyleSheet("NetCamView { background-color: #c1c1c1; }");
}

void NetCamView::show(VideoSource *source, const QString &title)
{
    setSource(source);
    //selfVideoSurface->setSource(&CameraSource::getInstance());
    setTitle(title);

    QWidget::show();
    //updateSize();
}

void NetCamView::hide()
{
    qDebug() << "jd";
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
#include <QPainter>
#include "src/widget/style.h"
void NetCamView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setBrush(Style::getColor(Style::ThemeDark));
    painter.drawRect(rect());
}

void NetCamView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateSize();
}

void NetCamView::updateSize()
{
    qDebug() << videoSurface->size();
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
    selfFrame->setBoundary(newSize, initial, newSize.width() / initialSize.width(), newSize.height() / initialSize.height());
}
