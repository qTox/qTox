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

#include "groupnetcamview.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/video/videosurface.h"
#include "src/persistence/settings.h"
#include "src/audio/audio.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/friend.h"
#include <QBoxLayout>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>
#include <QMap>

#include <QDebug>
class LabeledVideo : public QFrame
{
public:
    LabeledVideo(const QPixmap& avatar, QWidget* parent = 0, bool expanding = true)
        : QFrame(parent)
    {
        //setFrameStyle(QFrame::Box);
        qDebug() << "Created expanding? " << expanding;
        videoSurface = new VideoSurface(avatar, 0, expanding);
        videoSurface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        videoSurface->setMinimumHeight(32);
        //videoSurface->setMaximumHeight(96);
        connect(videoSurface, &VideoSurface::ratioChanged, this, &LabeledVideo::updateSize);
        label = new CroppingLabel(this);
        label->setTextFormat(Qt::PlainText);
        label->setStyleSheet("color: white");
        //label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //qDebug() << label->sizePolicy();
        label->setAlignment(Qt::AlignCenter);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(videoSurface, 1);
        layout->addWidget(label);
    }

    ~LabeledVideo()
    {

    }

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
        qDebug() << "Resize!";
        updateSize();
        QWidget::resizeEvent(event);
    }

private slots:
    void updateSize()
    {
        qDebug() << videoSurface->isExpanding();
        if (videoSurface->isExpanding())
        {
            int width = videoSurface->height() * videoSurface->getRatio();
            videoSurface->setMinimumWidth(width);
            videoSurface->setMaximumWidth(width);
            //setMaximumWidth(width + layout()->margin() * 2);
            qDebug() << videoSurface->minimumWidth();
        }
    }

private:
    CroppingLabel* label;
    VideoSurface* videoSurface;
    bool selected = false;
};

GroupNetCamView::GroupNetCamView(int group, QWidget *parent)
    : GenericNetCamView(parent)
    , group(group)
{
    videoLabelSurface = new LabeledVideo(QPixmap(), this, false);
    videoSurface = videoLabelSurface->getVideoSurface();
    //videoSurface->setExpanding(false);
    videoSurface->setMinimumHeight(256);
    videoSurface->setContentsMargins(6, 6, 6, 0);
    videoLabelSurface->setContentsMargins(0, 0, 0, 0);
    videoLabelSurface->layout()->setMargin(0);
    videoLabelSurface->setStyleSheet("QFrame { background-color: black; }");

    //verLayout->insertWidget(0, videoLabelSurface, 1);

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    verLayout->insertWidget(0, splitter, 1);
    splitter->addWidget(videoLabelSurface);
    splitter->setStyleSheet("QSplitter { background-color: black; } QSplitter::handle { background-color: black; }");

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    QWidget* widget = new QWidget(nullptr);
    scrollArea->setWidgetResizable(true);
    horLayout = new QHBoxLayout(widget);
    //FlowLayout* horLayout = new FlowLayout(widget);
    horLayout->addStretch(1);

    selfVideoSurface = new LabeledVideo(Settings::getInstance().getSavedAvatar(Core::getInstance()->getSelfId().toString()), this);
    horLayout->addWidget(selfVideoSurface);
    //horLayout->setAlignment(selfVideoSurface, Qt::AlignCenter | Qt::AlignHCenter);

    horLayout->addStretch(1);
    splitter->addWidget(scrollArea);
    scrollArea->setWidget(widget);
    //verLayout->insertWidget(1, scrollArea);
    //scrollArea->setMinimumHeight(selfVideoSurface->height());

    connect(&Audio::getInstance(), &Audio::groupAudioPlayed, this, &GroupNetCamView::groupAudioPlayed);

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, &GroupNetCamView::findActivePeer);
    timer->start();

    connect(Core::getInstance(), &Core::selfAvatarChanged, [this](const QPixmap& pixmap)
    {
        selfVideoSurface->getVideoSurface()->setAvatar(pixmap);
        findActivePeer();
    });
    connect(Core::getInstance(), &Core::usernameSet, [this](const QString& username)
    {
        selfVideoSurface->setText(username);
        findActivePeer();
    });
    connect(Core::getInstance(), &Core::friendAvatarChanged, this, &GroupNetCamView::friendAvatarChanged);

    selfVideoSurface->setText(Core::getInstance()->getUsername());
}

void GroupNetCamView::clearPeers()
{
    QList<int> keys = videoList.keys();

    for (int &i : keys)
        removePeer(i);
}

void GroupNetCamView::addPeer(int peer, const QString& name)
{
    QPixmap groupAvatar = Settings::getInstance().getSavedAvatar(Core::getInstance()->getGroupPeerToxId(group, peer).toString());
    LabeledVideo* labeledVideo = new LabeledVideo(groupAvatar, this);
    labeledVideo->setText(name);
    horLayout->insertWidget(horLayout->count() - 1, labeledVideo);
    PeerVideo peerVideo;
    peerVideo.video = labeledVideo;
    videoList.insert(peer, peerVideo);

    findActivePeer();
}

void GroupNetCamView::removePeer(int peer)
{
    auto peerVideo = videoList.find(peer);

    if (peerVideo != videoList.end())
    {
        LabeledVideo* labeledVideo = peerVideo.value().video;
        horLayout->removeWidget(labeledVideo);
        labeledVideo->deleteLater();
        videoList.remove(peer);

        findActivePeer();
    }
}

void GroupNetCamView::setActive(int peer)
{
    if (peer == -1)
    {
        videoLabelSurface->setText(selfVideoSurface->getText());
        activePeer = -1;
        return;
    }

    auto peerVideo = videoList.find(peer);

    if (peerVideo != videoList.end())
    {
        // When group video exists:
        // videoSurface->setSource(peerVideo.value()->getVideoSurface()->source);

        auto lastVideo = videoList.find(activePeer);

        if (lastVideo != videoList.end())
            lastVideo.value().video->setActive(false);

        LabeledVideo *labeledVideo = peerVideo.value().video;
        videoLabelSurface->setText(labeledVideo->getText());
        videoLabelSurface->getVideoSurface()->setAvatar(labeledVideo->getVideoSurface()->getAvatar());
        labeledVideo->setActive();

        activePeer = peer;
    }
}

void GroupNetCamView::groupAudioPlayed(int Group, int peer, unsigned short volume)
{
    if (group != Group)
        return;

    auto peerVideo = videoList.find(peer);

    if (peerVideo != videoList.end())
        peerVideo.value().volume = volume;
}

void GroupNetCamView::findActivePeer()
{
    int candidate = -1;
    int maximum = 0;

    for (auto peer = videoList.begin(); peer != videoList.end(); ++peer)
    {
        if (peer.value().volume > maximum)
        {
            maximum = peer.value().volume;
            candidate = peer.key();
        }
    }

    setActive(candidate);
}

void GroupNetCamView::friendAvatarChanged(int FriendId, const QPixmap &pixmap)
{
    Friend* f = FriendList::findFriend(FriendId);

    for (int i = 0; i < Core::getInstance()->getGroupNumberPeers(group); ++i)
    {
        if (Core::getInstance()->getGroupPeerToxId(group, i) == f->getToxId())
        {
            auto peerVideo = videoList.find(i);

            if (peerVideo != videoList.end())
            {
                peerVideo.value().video->getVideoSurface()->setAvatar(pixmap);
                findActivePeer();
            }

            break;
        }
    }
}
