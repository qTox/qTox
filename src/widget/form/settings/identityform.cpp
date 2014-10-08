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

#include "src/core.h"
#include "ui_identitysettings.h"
#include "identityform.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/croppinglabel.h"
#include <QLabel>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>

IdentityForm::IdentityForm() :
    GenericForm(tr("Your identity"), QPixmap(":/img/settings/identity.png"))
{
    bodyUI = new Ui::IdentitySettings;
    bodyUI->setupUi(this);

    // tox
    toxId = new ClickableTE();
    QFont small;
    small.setPixelSize(13);
    small.setKerning(false);

//    toxId->setTextInteractionFlags(Qt::TextSelectableByMouse);
    toxId->setReadOnly(true);
//    toxId->setFrameStyle(QFrame::NoFrame);
//    toxId->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    toxId->setFixedHeight(toxId->document()->size().height()*2);
    toxId->setFont(small);
    
    bodyUI->toxGroup->layout()->addWidget(toxId);
    
    connect(bodyUI->toxIdLabel, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(toxId, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(bodyUI->userName, SIGNAL(editingFinished()), this, SLOT(onUserNameEdited()));
    connect(bodyUI->statusMessage, SIGNAL(editingFinished()), this, SLOT(onStatusMessageEdited()));
}

IdentityForm::~IdentityForm()
{
}

void IdentityForm::copyIdClicked()
{
    toxId->selectAll();
    QString txt = toxId->text();
    txt.replace('\n',"");
    QApplication::clipboard()->setText(txt);
}

void IdentityForm::onUserNameEdited()
{
    Core::getInstance()->setUsername(bodyUI->userName->text());
}

void IdentityForm::onStatusMessageEdited()
{
    Core::getInstance()->setStatusMessage(bodyUI->statusMessage->text());
}

void IdentityForm::updateContent()
{
    toxId->setText(Core::getInstance()->getSelfId().toString());
}

void IdentityForm::setUserName(const QString &name)
{
    bodyUI->userName->setText(name);
}

void IdentityForm::setStatusMessage(const QString &msg)
{
    bodyUI->statusMessage->setText(msg);
}
