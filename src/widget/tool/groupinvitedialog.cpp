/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright (C) 2015 SylvieLorxu
    
    This file is part of Tox Qt GUI.
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    
    See the COPYING file for more details.
*/

#include "groupinvitedialog.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

GroupInviteDialog::GroupInviteDialog(QString friendUsername) :
    QDialog()
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Group invite","Title of the window to accept/deny a group invite"));

    QLabel *inviteLabel = new QLabel(tr("%1 invites you to a group chat.\nWould you like to join?").arg(friendUsername), this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);

    buttonBox->addButton(QDialogButtonBox::Yes);
    buttonBox->addButton(QDialogButtonBox::No);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &GroupInviteDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GroupInviteDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(inviteLabel);
    layout->addSpacing(12);
    layout->addWidget(buttonBox);
}
