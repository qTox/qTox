/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#pragma once

#include <QDialog>

namespace Ui {
class SetPasswordDialog;
}

class SetPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    enum ReturnCode
    {
        Rejected = QDialog::Rejected,
        Accepted = QDialog::Accepted,
        Tertiary
    };
    explicit SetPasswordDialog(QString body, QString extraButton, QWidget* parent = nullptr);
    ~SetPasswordDialog();
    QString getPassword();
    static int getPasswordStrength(QString pass);

private slots:
    void onPasswordEdit();

private:
    Ui::SetPasswordDialog* ui;
    QString body;
    static const double reasonablePasswordLength;
};
