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

#ifndef GROUPNETCAMVIEW_H
#define GROUPNETCAMVIEW_H

#include "genericnetcamview.h"

#include "src/core/toxpk.h"

#include <QMap>

class LabeledVideo;
class QHBoxLayout;

class GroupNetCamView : public GenericNetCamView
{
public:
    GroupNetCamView(int group, QWidget* parent = nullptr);
    void clearPeers();
    void addPeer(const ToxPk& peer, const QString& name);
    void removePeer(const ToxPk& peer);

private slots:
    void onUpdateActivePeer();
    void friendAvatarChanged(ToxPk friendPk, const QPixmap& pixmap);

private:
    struct PeerVideo
    {
        LabeledVideo* video;
    };

    void setActive(const ToxPk& peer = ToxPk{});

    QHBoxLayout* horLayout;
    QMap<ToxPk, PeerVideo> videoList;
    LabeledVideo* videoLabelSurface;
    LabeledVideo* selfVideoSurface;
    int activePeer;
    int group;
};

#endif // GROUPNETCAMVIEW_H
