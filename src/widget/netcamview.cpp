/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "netcamview.h"
#include "src/core/core.h"
#include "src/widget/videosurface.h"
#include <QLabel>
#include <QHBoxLayout>

NetCamView::NetCamView(QWidget* parent)
    : QWidget(parent)
    , mainLayout(new QHBoxLayout())
{
    setLayout(mainLayout);
    setWindowTitle(tr("Tox video"));
    setMinimumSize(320,240);

    videoSurface = new VideoSurface(this);

    mainLayout->addWidget(videoSurface);
}

void NetCamView::show(VideoSource *source, const QString &title)
{
    setSource(source);
    setTitle(title);

    QWidget::show();
}

void NetCamView::hide()
{
    setSource(nullptr);

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
