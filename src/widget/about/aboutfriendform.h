/*
    Copyright © 2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

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

signals:
    void histroyRemoved();

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
