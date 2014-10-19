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

#include "inputpassworddialog.h"
#include "ui_inputpassworddialog.h"

InputPasswordDialog::InputPasswordDialog(QString title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputPasswordDialog)
{
    ui->setupUi(this);

    if (title != QString())
        setWindowTitle(title);
}

InputPasswordDialog::~InputPasswordDialog()
{
    delete ui;
}

QString InputPasswordDialog::getPassword()
{
    return ui->passwordLineEdit->text();
}
