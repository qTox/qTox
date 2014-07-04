#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class GroupWidget : public QWidget
{
    Q_OBJECT
public:
    GroupWidget(int GroupId, QString Name);
    void onUserListChanged();
    void mouseReleaseEvent (QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent * event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);


signals:
    void groupWidgetClicked(GroupWidget* widget);
    void removeGroup(int groupId);

public:
    int groupId;
    QLabel avatar, name, nusers, statusPic;
    QHBoxLayout layout;
    QVBoxLayout textLayout;
    void setAsInactiveChatroom();
    void setAsActiveChatroom();
    void setNewFixedWidth(int newWidth);

private:
    QColor lastColor;
    int isActiveWidget;
};

#endif // GROUPWIDGET_H
