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

#ifndef DELETEFRIENDDIALOG_H
#define DELETEFRIENDDIALOG_H


#include "ui_removefrienddialog.h"
#include "src/model/friend.h"
#include <QDialog>


class RemoveFriendDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RemoveFriendDialog(QWidget* parent, const Friend* f);

    inline bool removeHistory()
    {
        return ui.removeHistory->isChecked();
    }

    inline bool accepted()
    {
        return _accepted;
    }

public slots:
    void onAccepted();

protected:
    Ui_RemoveFriendDialog ui;
    bool _accepted = false;
};

#endif // DELETEFRIENDDIALOG_H
