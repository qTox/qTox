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

#include "setpassworddialog.h"
#include "ui_setpassworddialog.h"
#include <QPushButton>

const double SetPasswordDialog::reasonablePasswordLength = 8.;

SetPasswordDialog::SetPasswordDialog(QString body, QString extraButton, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SetPasswordDialog)
    , body(body+"\n")
{
    ui->setupUi(this);

    connect(ui->  passwordlineEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEdit()));
    connect(ui->repasswordlineEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEdit()));

    ui->body->setText(body + "\n" + tr("The passwords don't match."));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    if (!extraButton.isEmpty())
    {
        QPushButton* third = new QPushButton(extraButton);
        ui->buttonBox->addButton(third, QDialogButtonBox::YesRole);
        connect(third, &QPushButton::clicked, this, [&](){this->done(Tertiary);});
    }
}

SetPasswordDialog::~SetPasswordDialog()
{
    delete ui;
}

void SetPasswordDialog::onPasswordEdit()
{
    QString pswd = ui->passwordlineEdit->text();


    if (pswd.length() < 6)
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body + tr("The password is too short"));
    }
    else if (pswd != ui->repasswordlineEdit->text())
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body + tr("The password doesn't match."));
    }
    else
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->body->setText(body);
    }

    // Password strength calculator
    // Based on code in the Master Password dialog in Firefox
    // (pref-masterpass.js)
    // Original code triple-licensed under the MPL, GPL, and LGPL
    // so is license-compatible with this file

    const double lengthFactor = reasonablePasswordLength / 8.0;
    int pwlength = (int)(pswd.length() / lengthFactor);
    if (pwlength > 5)
        pwlength = 5;

    const QRegExp numRxp("[0-9]", Qt::CaseSensitive, QRegExp::RegExp);
    int numeric = (int)(pswd.count(numRxp) / lengthFactor);
    if (numeric > 3)
        numeric = 3;

    const QRegExp symbRxp("\\W", Qt::CaseInsensitive, QRegExp::RegExp);
    int numsymbols = (int)(pswd.count(symbRxp) / lengthFactor);
    if (numsymbols > 3)
        numsymbols = 3;

    const QRegExp upperRxp("[A-Z]", Qt::CaseSensitive, QRegExp::RegExp);
    int upper = (int)(pswd.count(upperRxp) / lengthFactor);
    if (upper > 3)
        upper = 3;

    int pwstrength=((pwlength*10)-20) + (numeric*10) + (numsymbols*15) + (upper*10);
    if (pwstrength < 0)
        pwstrength = 0;

    if (pwstrength > 100)
        pwstrength = 100;

    ui->strengthBar->setValue(pwstrength);
}

QString SetPasswordDialog::getPassword()
{
    return ui->passwordlineEdit->text();
}
