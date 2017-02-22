#ifndef GROUPINVITEWIDGET_H
#define GROUPINVITEWIDGET_H

#include "src/groupinvite.h"

#include <QWidget>

class CroppingLabel;

class QHBoxLayout;
class QPushButton;

class GroupInviteWidget : public QWidget
{
    Q_OBJECT
public:
    GroupInviteWidget(QWidget* parent, const GroupInvite& invite);
    void retranslateUi();
    const GroupInvite getInviteInfo() const;

signals:
    void accepted(const GroupInvite& invite);
    void rejected(const GroupInvite& invite);

private:
    QPushButton* acceptButton;
    QPushButton* rejectButton;
    CroppingLabel* inviteMessageLabel;
    QHBoxLayout* widgetLayout;
    GroupInvite inviteInfo;
};

#endif // GROUPINVITEWIDGET_H
