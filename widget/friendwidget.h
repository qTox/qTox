#ifndef FRIENDWIDGET_H
#define FRIENDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

struct FriendWidget : public QWidget
{
    Q_OBJECT
public:
    FriendWidget(int FriendId, QString id);
    void mouseReleaseEvent (QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent * event);
    void setAsActiveChatroom();
    void setAsInactiveChatroom();

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void removeFriend(int friendId);

public:
    int friendId;
    QLabel avatar, name, statusMessage, statusPic;
    QHBoxLayout layout;
    QVBoxLayout textLayout;
    void setNotificationLight();
};

#endif // FRIENDWIDGET_H
