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
#include "src/widget/style.h"

#include <QApplication>
#include <QBoxLayout>
#include <QScreen>
#include <QFrame>
#include <QLabel>
#include <QCloseEvent>
#include <QPushButton>
#include <QDesktopWidget>

namespace
{
const auto BTN_STATE_NONE = QVariant("none");
const auto BTN_STATE_RED = QVariant("red");
const int BTN_PANEL_HEIGHT = 55;
const int BTN_PANEL_WIDTH = 250;
const auto BTN_STYLE_SHEET_PATH = QStringLiteral("chatForm/fullScreenButtons.css");
}

NetCamView::NetCamView(ToxPk friendPk, QWidget* parent)
    : selfFrame{nullptr}
    , friendPk{friendPk}
    , e(false)
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

    connect(toggleMessagesButton, &QPushButton::clicked, this, &NetCamView::showMessageClicked);
    connect(enterFullScreenButton, &QPushButton::clicked, this, &NetCamView::toggleFullScreen);

    verLayout->addLayout(buttonLayout);
    verLayout->setContentsMargins(0, 0, 0, 0);

    setShowMessages(false);

    setStyleSheet("NetCamView { background-color: #c1c1c1; }");
    buttonPanel = new QFrame(this);
    buttonPanel->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));
    buttonPanel->setGeometry(0, 0, BTN_PANEL_WIDTH, BTN_PANEL_HEIGHT);

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

    connect(videoPreviewButton, &QPushButton::clicked, this, &NetCamView::toggleVideoPreview);
    connect(volumeButton, &QPushButton::clicked, this, &NetCamView::volMuteToggle);
    connect(microphoneButton, &QPushButton::clicked, this, &NetCamView::micMuteToggle);
    connect(endVideoButton, &QPushButton::clicked, this, &NetCamView::endVideoCall);
    connect(exitFullScreenButton, &QPushButton::clicked, this, &NetCamView::toggleFullScreen);

    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(videoPreviewButton);
    buttonPanelLayout->addWidget(volumeButton);
    buttonPanelLayout->addWidget(microphoneButton);
    buttonPanelLayout->addWidget(endVideoButton);
    buttonPanelLayout->addWidget(exitFullScreenButton);
    buttonPanelLayout->addStretch();

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
    Q_UNUSED(event)
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

QSize NetCamView::getSurfaceMinSize()
{
    QSize surfaceSize = videoSurface->minimumSize();
    QSize buttonSize = toggleMessagesButton->size();
    QSize panelSize(0, 45);

    return surfaceSize + buttonSize + panelSize;
}

void NetCamView::setShowMessages(bool show, bool notify)
{
    if (!show) {
        toggleMessagesButton->setText(tr("Hide messages"));
        toggleMessagesButton->setIcon(QIcon());
        return;
    }

    toggleMessagesButton->setText(tr("Show messages"));

    if (notify) {
        toggleMessagesButton->setIcon(QIcon(Style::getImagePath("chatArea/info.svg")));
    }
}

void NetCamView::toggleFullScreen()
{
    if (isFullScreen()) {
        exitFullScreen();
    } else {
        enterFullScreen();
    }
}

void NetCamView::enterFullScreen()
{
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    showFullScreen();
    enterFullScreenButton->hide();
    toggleMessagesButton->hide();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const auto screenSize = QGuiApplication::screenAt(this->pos())->geometry();
#else
    const QRect screenSize = QApplication::desktop()->screenGeometry(this);
#endif
    buttonPanel->setGeometry((screenSize.width() / 2) - buttonPanel->width() / 2,
            screenSize.height() - BTN_PANEL_HEIGHT - 25, BTN_PANEL_WIDTH, BTN_PANEL_HEIGHT);
    buttonPanel->show();
    buttonPanel->activateWindow();
    buttonPanel->raise();
}

void NetCamView::exitFullScreen()
{
    setWindowFlags(Qt::Widget);
    showNormal();
    buttonPanel->hide();
    enterFullScreenButton->show();
    toggleMessagesButton->show();
}

void NetCamView::endVideoCall()
{
    toggleFullScreen();
    emit videoCallEnd();
}

void NetCamView::toggleVideoPreview()
{
    toggleButtonState(videoPreviewButton);
    if (selfFrame->isHidden()) {
        selfFrame->show();
    } else {
        selfFrame->hide();
    }
}

QPushButton* NetCamView::createButton(const QString& name, const QString& state)
{
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", QVariant(state));
    btn->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));

    return btn;
}

void NetCamView::updateMuteVolButton(bool isMuted)
{
    updateButtonState(volumeButton, !isMuted);
}

void NetCamView::updateMuteMicButton(bool isMuted)
{
    updateButtonState(microphoneButton, !isMuted);
}

void NetCamView::toggleButtonState(QPushButton* btn)
{
    if (btn->property("state") == BTN_STATE_RED) {
        btn->setProperty("state", BTN_STATE_NONE);
    } else {
        btn->setProperty("state", BTN_STATE_RED);
    }

    btn->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));
}

void NetCamView::updateButtonState(QPushButton* btn, bool active)
{
    if (active) {
        btn->setProperty("state", BTN_STATE_NONE);
    } else {
        btn->setProperty("state", BTN_STATE_RED);
    }

    btn->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));
}

void NetCamView::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Escape && isFullScreen()) {
        exitFullScreen();
    }
}

void NetCamView::closeEvent(QCloseEvent *event)
{
    exitFullScreen();
    event->ignore();
}
