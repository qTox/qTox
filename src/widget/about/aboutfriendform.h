#ifndef ABOUT_USER_FORM_H
#define ABOUT_USER_FORM_H

#include "src/model/friend.h"
#include <QDialog>

namespace Ui {
class AboutFriendForm;
}

class AboutFriendForm : public QDialog
{
    Q_OBJECT

public:
    explicit AboutFriendForm(const Friend* f, QWidget* parent = 0);
    ~AboutFriendForm();

private:
    Ui::AboutFriendForm* ui;
    ToxPk friendPk;

private slots:
    void onAcceptedClicked();
    void onAutoAcceptDirClicked();
    void onAutoAcceptCallClicked();
    void onAutoGroupInvite();
    void onSelectDirClicked();
    void onRemoveHistoryClicked();
};

#endif // ABOUT_USER_FORM_H
