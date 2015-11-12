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
    explicit AboutUser(QWidget *parent = 0);
    ~AboutUser();
    void setFriend(Friend *f);
    void setToxId(ToxId &id);

private:
    Ui::AboutUser *ui;
    ToxId toxId;

private slots:
    void onAcceptedClicked();
};

#endif // ABOUTUSER_H
