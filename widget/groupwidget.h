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
    void contextMenuEvent(QContextMenuEvent * event);

signals:
    void groupWidgetClicked(GroupWidget* widget);
    void removeGroup(int groupId);

public:
    int groupId;
    QLabel avatar, name, nusers;
    QHBoxLayout layout;
    QVBoxLayout textLayout;
};

#endif // GROUPWIDGET_H
