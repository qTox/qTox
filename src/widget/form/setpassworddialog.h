/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef SETPASSWORDDIALOG_H
#define SETPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class SetPasswordDialog;
}

class SetPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    enum ReturnCode {Rejected=QDialog::Rejected, Accepted=QDialog::Accepted, Tertiary};
    explicit SetPasswordDialog(QString body, QString extraButton, QWidget* parent = 0);
    ~SetPasswordDialog();
    QString getPassword();

private slots:
    void onPasswordEdit();

private:
    Ui::SetPasswordDialog *ui;
    QString body;
    static const double reasonablePasswordLength;
};

#endif // SETPASSWORDDIALOG_H
