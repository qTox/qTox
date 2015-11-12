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
    explicit AboutUser(ToxId &toxID, QWidget *parent = 0);
    ~AboutUser();
    void setFriend(Friend *f);

private:
    Ui::AboutUser *ui;
    ToxId toxId;

private slots:
    void onAcceptedClicked();
    void onAutoAcceptClicked();
    void onSelectDirClicked();
};

#endif // ABOUTUSER_H
