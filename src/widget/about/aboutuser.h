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
    explicit AboutUser(const Friend* f, QWidget* parent = 0);
    ~AboutUser();

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
