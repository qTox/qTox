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


#ifndef ANDROIDGUI_H
#define ANDROIDGUI_H

#include "src/core/corestructs.h"
#include <QWidget>

class MaskablePixmapWidget;
class FriendListWidget;
class QKeyEvent;

namespace Ui {
class Android;
}

class AndroidGUI : public QWidget
{
    Q_OBJECT
public:
    explicit AndroidGUI(QWidget *parent = 0);
    ~AndroidGUI();

public slots:
    void onConnected();
    void onDisconnected();
    void onStatusSet(Status status);
    void onSelfAvatarLoaded(const QPixmap &pic);
    void setUsername(const QString& username);
    void setStatusMessage(const QString &statusMessage);

signals:
    void friendRequestAccepted(const QString& userId);
    void friendRequested(const QString& friendAddress, const QString& message);
    void statusSet(Status status);
    void statusSelected(Status status);
    void usernameChanged(const QString& username);
    void statusMessageChanged(const QString& statusMessage);

private:
    void reloadTheme();
    virtual void keyPressEvent(QKeyEvent* event) final override;

private:
    Ui::Android* ui;
    MaskablePixmapWidget* profilePicture;
    FriendListWidget* contactListWidget;
    Status beforeDisconnect = Status::Offline;
    QRegExp nameMention, sanitizedNameMention;
};

#endif // ANDROIDGUI_H
