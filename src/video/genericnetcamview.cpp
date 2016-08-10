/*
    Copyright © 2015 by The qTox Project

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

#include "genericnetcamview.h"

#include <QBoxLayout>
#include <QFrame>
#include <QPushButton>

GenericNetCamView::GenericNetCamView(QWidget *parent)
    : QWidget(parent)
{
    verLayout = new QVBoxLayout(this);
    setWindowTitle(tr("Tox video"));

    int spacing = verLayout->spacing();
    verLayout->setSpacing(0);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    button = new QPushButton();
    buttonLayout->addWidget(button);
    buttonLayout->setSizeConstraint(QLayout::SetMinimumSize);
    connect(button, &QPushButton::clicked, this, &GenericNetCamView::showMessageClicked);

    verLayout->addSpacing(spacing);
    verLayout->addLayout(buttonLayout);
    verLayout->addSpacing(spacing);

    QFrame* lineFrame = new QFrame(this);
    lineFrame->setStyleSheet("border: 1px solid #c1c1c1;");
    lineFrame->setFrameShape(QFrame::HLine);
    lineFrame->setMaximumHeight(1);
    verLayout->addWidget(lineFrame);

    setShowMessages(false);

    setStyleSheet("NetCamView { background-color: #c1c1c1; }");
}

QSize GenericNetCamView::getSurfaceMinSize()
{
    QSize surfaceSize = videoSurface->minimumSize();
    QSize buttonSize = button->size();
    QSize panelSize(0, 45);

    return surfaceSize + buttonSize + panelSize;
}

void GenericNetCamView::setShowMessages(bool show, bool notify)
{
    if (!show)
    {
        button->setText(tr("Hide Messages"));
        button->setIcon(QIcon());
        return;
    }

    button->setText(tr("Show Messages"));

    if (notify)
        button->setIcon(QIcon(":/ui/chatArea/info.svg"));
}
