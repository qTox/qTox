#ifndef ABOUT_USER_FORM_H
#define ABOUT_USER_FORM_H

#include "src/model/about/iaboutfriend.h"

#include <QDialog>
#include <QPointer>

#include <memory>

namespace Ui {
class AboutFriendForm;
}

class AboutFriendForm : public QDialog
{
    Q_OBJECT

public:
    AboutFriendForm(std::unique_ptr<IAboutFriend> about, QWidget* parent = nullptr);
    ~AboutFriendForm();

private:
    Ui::AboutFriendForm* ui;
    const std::unique_ptr<IAboutFriend> about;

private slots:
    void onAutoAcceptDirChanged(const QString& path);
    void onAcceptedClicked();
    void onAutoAcceptDirClicked();
    void onAutoAcceptCallClicked();
    void onAutoGroupInvite();
    void onSelectDirClicked();
    void onRemoveHistoryClicked();
};

#endif // ABOUT_USER_FORM_H
