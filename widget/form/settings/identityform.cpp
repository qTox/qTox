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
#include "widget/croppinglabel.h"
#include "core.h"
#include <QLabel>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>

IdentityForm::IdentityForm()
{
    icon.setPixmap(QPixmap(":/img/settings/identity.png").scaledToHeight(headLayout.sizeHint().height(), Qt::SmoothTransformation));
    label.setText(tr("Your identity"));

    // public
    publicGroup = new QGroupBox(tr("Public Information"));
    userNameLabel = new QLabel(tr("Name","Username/nick"));
    userName = new QLineEdit();
    
    statusMessageLabel = new QLabel(tr("Status","Status message"));
    statusMessage = new QLineEdit();
    
    vLayout = new QVBoxLayout();
    vLayout->addWidget(userNameLabel);
    vLayout->addWidget(userName);
    vLayout->addWidget(statusMessageLabel);
    vLayout->addWidget(statusMessage);
    publicGroup->setLayout(vLayout);
    
    // tox
    toxGroup = new QGroupBox(tr("Tox ID"));
    toxIdLabel = new CroppingLabel();
    toxIdLabel->setText(tr("Your Tox ID (click to copy)"));
    toxId = new ClickableTE();
    QFont small;
    small.setPixelSize(13);
    small.setKerning(false);
    toxId->setTextInteractionFlags(Qt::TextSelectableByMouse);
    toxId->setReadOnly(true);
    toxId->setFrameStyle(QFrame::NoFrame);
    toxId->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    toxId->setFixedHeight(toxId->document()->size().height()*2);
    toxId->setFont(small);
    
    QVBoxLayout* toxLayout = new QVBoxLayout();
    toxLayout->addWidget(toxIdLabel);
    toxLayout->addWidget(toxId);
    toxGroup->setLayout(toxLayout);
    
    layout.setSpacing(30);
    layout.addWidget(publicGroup);
    layout.addWidget(toxGroup);
    layout.addStretch(1);
    
    connect(toxIdLabel, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(toxId, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(userName, SIGNAL(editingFinished()), this, SLOT(onUserNameEdited()));
    connect(statusMessage, SIGNAL(editingFinished()), this, SLOT(onStatusMessageEdited()));
}

IdentityForm::~IdentityForm()
{
}

void IdentityForm::copyIdClicked()
{
    toxId->selectAll();
    QString txt = toxId->toPlainText();
    txt.replace('\n',"");
    QApplication::clipboard()->setText(txt);
}

void IdentityForm::onUserNameEdited()
{
    emit userNameChanged(userName->text());
}

void IdentityForm::onStatusMessageEdited()
{
    emit statusMessageChanged(statusMessage->text());
}

void IdentityForm::show(SettingsWidget& sw)
{
    userName->setText(Core::getInstance()->getUsername());
    statusMessage->setText(Core::getInstance()->getStatusMessage());
    toxId->setText(Core::getInstance()->getSelfId().toString());
    GenericForm::show(sw);
}
