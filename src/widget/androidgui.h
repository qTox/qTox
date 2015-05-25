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
    virtual void keyPressEvent(QKeyEvent* event) final;

private slots:
    void onUsernameChanged(const QString& newUsername, const QString& oldUsername);
    void onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage);

private:
    Ui::Android* ui;
    MaskablePixmapWidget* profilePicture;
    FriendListWidget* contactListWidget;
    Status beforeDisconnect = Status::Offline;
    QRegExp nameMention, sanitizedNameMention;
};

#endif // ANDROIDGUI_H
