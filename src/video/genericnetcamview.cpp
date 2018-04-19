/*
    Copyright © 2015 by The qTox Project Contributors

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
#include <QKeyEvent>
#include <QPushButton>
#include <QTimer>

GenericNetCamView::GenericNetCamView(QWidget* parent)
    : QWidget(parent)
{
    verLayout = new QVBoxLayout(this);
    setWindowTitle(tr("Tox video"));

    buttonLayout = new QHBoxLayout();

    toggleMessagesButton = new QPushButton();
    enterFullScreenButton = new QPushButton();
    enterFullScreenButton->setText(tr("Full Screen"));

    buttonLayout->addStretch();
    buttonLayout->addWidget(toggleMessagesButton);
    buttonLayout->addWidget(enterFullScreenButton);

    connect(toggleMessagesButton, &QPushButton::clicked, this, &GenericNetCamView::showMessageClicked);
    connect(enterFullScreenButton, &QPushButton::clicked, this, &GenericNetCamView::toggleFullScreen);

    verLayout->addLayout(buttonLayout);
    verLayout->setContentsMargins(0, 0, 0, 0);

    setShowMessages(false);

    setStyleSheet("NetCamView { background-color: #c1c1c1; }");
    buttonPanel = new QFrame(this);
    buttonPanel->setStyleSheet(Style::getStylesheet(buttonsStyleSheetPath));
    buttonPanel->setGeometry(0, 0, buttonPanelWidth, buttonPanelHeight);

    QHBoxLayout* buttonPanelLayout = new QHBoxLayout(buttonPanel);
    buttonPanelLayout->setContentsMargins(20, 0, 20, 0);

    videoPreviewButton = createButton("videoPreviewButton", "none");
    videoPreviewButton->setToolTip(tr("Toggle video preview"));

    volumeButton = createButton("volButtonFullScreen", "none");
    volumeButton->setToolTip(tr("Mute audio"));

    microphoneButton = createButton("micButtonFullScreen", "none");
    microphoneButton->setToolTip(tr("Mute microphone"));

    endVideoButton = createButton("videoButtonFullScreen", "none");
    endVideoButton->setToolTip(tr("End video call"));

    exitFullScreenButton = createButton("exitFullScreenButton", "none");
    exitFullScreenButton->setToolTip(tr("Exit full screen"));

    connect(videoPreviewButton, &QPushButton::clicked, this, &GenericNetCamView::toggleVideoPreview);
    connect(volumeButton, SIGNAL(clicked()), this, SIGNAL(volMuteToggle()));
    connect(microphoneButton, SIGNAL(clicked()), this, SIGNAL(micMuteToggle()));
    connect(endVideoButton, &QPushButton::clicked, this, &GenericNetCamView::endVideoCall);
    connect(exitFullScreenButton, &QPushButton::clicked, this, &GenericNetCamView::toggleFullScreen);

    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(videoPreviewButton);
    buttonPanelLayout->addWidget(volumeButton);
    buttonPanelLayout->addWidget(microphoneButton);
    buttonPanelLayout->addWidget(endVideoButton);
    buttonPanelLayout->addWidget(exitFullScreenButton);
    buttonPanelLayout->addStretch();
}

QSize GenericNetCamView::getSurfaceMinSize()
{
    QSize surfaceSize = videoSurface->minimumSize();
    QSize buttonSize = toggleMessagesButton->size();
    QSize panelSize(0, 45);

    return surfaceSize + buttonSize + panelSize;
}

void GenericNetCamView::setShowMessages(bool show, bool notify)
{
    if (!show) {
        toggleMessagesButton->setText(tr("Hide Messages"));
        toggleMessagesButton->setIcon(QIcon());
        return;
    }

    toggleMessagesButton->setText(tr("Show Messages"));

    if (notify) {
        toggleMessagesButton->setIcon(QIcon(":/ui/chatArea/info.svg"));
    }
}

void GenericNetCamView::toggleFullScreen()
{
    if (isFullScreen()) {
        exitFullScreen();
    } else {
        enterFullScreen();
    }
}

void GenericNetCamView::enterFullScreen()
{
    setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    showFullScreen();
    enterFullScreenButton->hide();
    toggleMessagesButton->hide();

    // we need to get the correct width and height but it has to be
    // after the widget switched to full screen - there must be a better way?
    QTimer::singleShot(200, this, [this]() {
        buttonPanel->setGeometry((width() / 2) - buttonPanel->width() / 2,
                height() - buttonPanelHeight - 25, buttonPanelWidth, buttonPanelHeight);
        buttonPanel->show();
        buttonPanel->activateWindow();
        buttonPanel->raise();
    });
}

void GenericNetCamView::exitFullScreen()
{
    setWindowFlags(Qt::Widget);
    showNormal();
    buttonPanel->hide();
    enterFullScreenButton->show();
    toggleMessagesButton->show();
}

void GenericNetCamView::endVideoCall()
{
    toggleFullScreen();
    emit videoCallEnd();
}

void GenericNetCamView::toggleVideoPreview()
{
    toggleButtonState(videoPreviewButton);
    emit videoPreviewToggle();
}

QPushButton *GenericNetCamView::createButton(const QString& name, const QString& state)
{
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", QVariant(state));
    btn->setStyleSheet(Style::getStylesheet(buttonsStyleSheetPath));

    return btn;
}

void GenericNetCamView::updateMuteVolButton(bool isMuted)
{
    updateButtonState(volumeButton, !isMuted);
}

void GenericNetCamView::updateMuteMicButton(bool isMuted)
{
    updateButtonState(microphoneButton, !isMuted);
}

void GenericNetCamView::toggleButtonState(QPushButton* btn)
{
    if (btn->property("state") == btnStateRed) {
        btn->setProperty("state", btnStateNone);
    } else {
        btn->setProperty("state", btnStateRed);
    }

    btn->setStyleSheet(Style::getStylesheet(buttonsStyleSheetPath));
}

void GenericNetCamView::updateButtonState(QPushButton* btn, bool active)
{
    if (active) {
        btn->setProperty("state", btnStateNone);
    } else {
        btn->setProperty("state", btnStateRed);
    }

    btn->setStyleSheet(Style::getStylesheet(buttonsStyleSheetPath));
}

void GenericNetCamView::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Escape && isFullScreen()) {
        exitFullScreen();
    }
}
