#ifndef ABOUTUSER_H
#define ABOUTUSER_H

#include <QDialog>
#include "src/friend.h"


namespace Ui {
class AboutUser;
}

class AboutUser : public QDialog
{
    Q_OBJECT

public:
    explicit AboutUser(ToxKey &toxID, QWidget *parent = 0);
    ~AboutUser();
    void setFriend(Friend *f);

private:
    Ui::AboutUser *ui;
    ToxKey friendPk;

private slots:
    void onAcceptedClicked();
    void onAutoAcceptDirClicked();
    void onAutoAcceptCallClicked();
    void onSelectDirClicked();
    void onRemoveHistoryClicked();
};

#endif // ABOUTUSER_H
