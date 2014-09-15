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

#include "identityform.h"
#include "widget/form/settingswidget.h"
#include <QLabel>
#include <QLineEdit>

IdentityForm::IdentityForm()
{
    prep();
    icon.addFile(":/img/settings/identity.png");
    label.setText(tr("Your identity"));
    toxGroup = new QGroupBox(tr("Tox ID"));
    QLabel* toxIdLabel = new QLabel(tr("Your Tox ID"));
    QLineEdit* toxID = new QLineEdit();
    toxID->setReadOnly(true);
    QVBoxLayout* toxLayout = new QVBoxLayout();
    toxLayout->addWidget(toxIdLabel);
    toxLayout->addWidget(toxID);
    toxGroup->setLayout(toxLayout);
    layout.addWidget(toxGroup);
}

IdentityForm::~IdentityForm()
{
}
