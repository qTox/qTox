/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

SetPasswordDialog::SetPasswordDialog(QString body, QString extraButton, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SetPasswordDialog)
{
    ui->setupUi(this);

    connect(ui->passwordlineEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEdit()));
    connect(ui->repasswordlineEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEdit()));

    ui->body->setText(body + tr("\nTo encourage good habits, qTox requires at least 8 characters."));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    if (!extraButton.isEmpty())
    {
        QPushButton* third = new QPushButton(extraButton);
        ui->buttonBox->addButton(third, QDialogButtonBox::YesRole);
        connect(third, &QPushButton::clicked, this, [=](){this->done(2);});
    }
}

SetPasswordDialog::~SetPasswordDialog()
{
    delete ui;
}

void SetPasswordDialog::onPasswordEdit()
{
    if (   ui->passwordlineEdit->text().length() >= 8
        && ui->passwordlineEdit->text() == ui->repasswordlineEdit->text())
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

QString SetPasswordDialog::getPassword()
{
    return ui->passwordlineEdit->text();
}
