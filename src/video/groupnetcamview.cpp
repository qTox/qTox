/*
    Copyright © 2015-2018 by The qTox Project Contributors

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

#include "groupnetcamview.h"
#include "src/audio/audio.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/video/videosurface.h"
#include "src/widget/tool/croppinglabel.h"
#include <QBoxLayout>
#include <QMap>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>

#include <QDebug>
class LabeledVideo : public QFrame
{
public:
    LabeledVideo(const QPixmap& avatar, QWidget* parent = 0, bool expanding = true)
        : QFrame(parent)
    {
        qDebug() << "Created expanding? " << expanding;
        videoSurface = new VideoSurface(avatar, 0, expanding);
        videoSurface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        videoSurface->setMinimumHeight(32);

        connect(videoSurface, &VideoSurface::ratioChanged, this, &LabeledVideo::updateSize);
        label = new CroppingLabel(this);
        label->setTextFormat(Qt::PlainText);
        label->setStyleSheet("color: white");

        label->setAlignment(Qt::AlignCenter);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(videoSurface, 1);
        layout->addWidget(label);
    }

    ~LabeledVideo() {}

    VideoSurface* getVideoSurface() const
    {
        return videoSurface;
    }

    void setText(const QString& text)
    {
        label->setText(text);
    }

    QString getText() const
    {
        return label->text();
    }

    void setActive(bool active = true)
    {
        if (active)
            setStyleSheet("QFrame { background-color: #414141; border-radius: 10px; }");
        else
            setStyleSheet(QString());
    }

protected:
    void resizeEvent(QResizeEvent* event) final override
    {
        updateSize();
        QWidget::resizeEvent(event);
    }

private slots:
    void updateSize()
    {
        if (videoSurface->isExpanding()) {
            int width = videoSurface->height() * videoSurface->getRatio();
            videoSurface->setMinimumWidth(width);
            videoSurface->setMaximumWidth(width);
        }
    }

private:
    CroppingLabel* label;
    VideoSurface* videoSurface;
};

GroupNetCamView::GroupNetCamView(int group, QWidget* parent)
    : GenericNetCamView(parent)
    , group(group)
{
    videoLabelSurface = new LabeledVideo(QPixmap(), this, false);
    videoSurface = videoLabelSurface->getVideoSurface();
    videoSurface->setMinimumHeight(256);
    videoSurface->setContentsMargins(6, 6, 6, 0);
    videoLabelSurface->setContentsMargins(0, 0, 0, 0);
    videoLabelSurface->layout()->setMargin(0);
    videoLabelSurface->setStyleSheet("QFrame { background-color: black; }");

    // remove full screen button in audio group chat since it's useless there
    enterFullScreenButton->hide();

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);
    verLayout->insertWidget(0, splitter, 1);
    splitter->addWidget(videoLabelSurface);
    splitter->setStyleSheet(
        "QSplitter { background-color: black; } QSplitter::handle { background-color: black; }");

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    QWidget* widget = new QWidget(nullptr);
    scrollArea->setWidgetResizable(true);
    horLayout = new QHBoxLayout(widget);
    horLayout->addStretch(1);

    selfVideoSurface = new LabeledVideo(Nexus::getProfile()->loadAvatar(), this);
    horLayout->addWidget(selfVideoSurface);

    horLayout->addStretch(1);
    splitter->addWidget(scrollArea);
    scrollArea->setWidget(widget);

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, &GroupNetCamView::onUpdateActivePeer);
    timer->start();

    connect(Nexus::getProfile(), &Profile::selfAvatarChanged, [this](const QPixmap& pixmap) {
        selfVideoSurface->getVideoSurface()->setAvatar(pixmap);
        setActive();
    });
    connect(Core::getInstance(), &Core::usernameSet, [this](const QString& username) {
        selfVideoSurface->setText(username);
        setActive();
    });

    connect(Core::getInstance(), &Core::friendAvatarChangedDeprecated, this,
            &GroupNetCamView::friendAvatarChanged);

    selfVideoSurface->setText(Core::getInstance()->getUsername());
}

void GroupNetCamView::clearPeers()
{
    for (const auto& peerPk : videoList.keys()) {
        removePeer(peerPk);
    }
}

void GroupNetCamView::addPeer(const ToxPk& peer, const QString& name)
{
    QPixmap groupAvatar = Nexus::getProfile()->loadAvatar(peer);
    LabeledVideo* labeledVideo = new LabeledVideo(groupAvatar, this);
    labeledVideo->setText(name);
    horLayout->insertWidget(horLayout->count() - 1, labeledVideo);
    PeerVideo peerVideo;
    peerVideo.video = labeledVideo;
    videoList.insert(peer, peerVideo);

    setActive();
}

void GroupNetCamView::removePeer(const ToxPk& peer)
{
    auto peerVideo = videoList.find(peer);

    if (peerVideo != videoList.end()) {
        LabeledVideo* labeledVideo = peerVideo.value().video;
        horLayout->removeWidget(labeledVideo);
        labeledVideo->deleteLater();
        videoList.remove(peer);

        setActive();
    }
}

void GroupNetCamView::onUpdateActivePeer()
{
    setActive();
}

void GroupNetCamView::setActive(const ToxPk& peer)
{
    if (peer.isEmpty()) {
        videoLabelSurface->setText(selfVideoSurface->getText());
        activePeer = -1;
        return;
    }

    // TODO(sudden6): check if we can remove the code, it won't be reached right now
#if 0
    auto peerVideo = videoList.find(peer);

    if (peerVideo != videoList.end()) {
        // When group video exists:
        // videoSurface->setSource(peerVideo.value()->getVideoSurface()->source);

        auto lastVideo = videoList.find(activePeer);

        if (lastVideo != videoList.end())
            lastVideo.value().video->setActive(false);

        LabeledVideo* labeledVideo = peerVideo.value().video;
        videoLabelSurface->setText(labeledVideo->getText());
        videoLabelSurface->getVideoSurface()->setAvatar(labeledVideo->getVideoSurface()->getAvatar());
        labeledVideo->setActive();

        activePeer = peer;
    }
#endif
}

void GroupNetCamView::friendAvatarChanged(int friendId, const QPixmap& pixmap)
{
    const auto friendPk = Core::getInstance()->getFriendPublicKey(friendId);
    auto peerVideo = videoList.find(friendPk);

    if (peerVideo != videoList.end()) {
        peerVideo.value().video->getVideoSurface()->setAvatar(pixmap);
        setActive();
    }
}
