#ifndef ABOUTUSER_H
#define ABOUTUSER_H

#include "src/friend.h"
#include <QDialog>


namespace Ui {
class AboutUser;
}

class AboutUser : public QDialog
{
    Q_OBJECT

public:
    explicit AboutUser(ToxPk& toxID, QWidget* parent = 0);
    ~AboutUser();
    void setFriend(Friend* f);

private:
    Ui::AboutUser* ui;
    ToxPk friendPk;

private slots:
    void onAcceptedClicked();
    void onAutoAcceptDirClicked();
    void onAutoAcceptCallClicked();
    void onAutoGroupInvite();
    void onSelectDirClicked();
    void onRemoveHistoryClicked();
};

#endif // ABOUTUSER_H
