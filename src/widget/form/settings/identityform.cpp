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
#include "src/misc/settings.h"
#include "src/widget/croppinglabel.h"
#include "src/widget/widget.h"
#include "src/misc/style.h"
#include <QLabel>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

IdentityForm::IdentityForm() :
    GenericForm(tr("Identity"), QPixmap(":/img/settings/identity.png"))
{
    bodyUI = new Ui::IdentitySettings;
    bodyUI->setupUi(this);

    // tox
    toxId = new ClickableTE();
    toxId->setReadOnly(true);
    toxId->setFrame(false);
    toxId->setFont(Style::getFont(Style::Small));
    
    bodyUI->toxGroup->layout()->addWidget(toxId);
    
    connect(bodyUI->toxIdLabel, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(toxId, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(Core::getInstance(), &Core::idSet, this, &IdentityForm::setToxId);
    connect(bodyUI->userName, SIGNAL(editingFinished()), this, SLOT(onUserNameEdited()));
    connect(bodyUI->statusMessage, SIGNAL(editingFinished()), this, SLOT(onStatusMessageEdited()));
    connect(bodyUI->loadButton, &QPushButton::clicked, this, &IdentityForm::onLoadClicked);
    connect(bodyUI->renameButton, &QPushButton::clicked, this, &IdentityForm::onRenameClicked);
    connect(bodyUI->exportButton, &QPushButton::clicked, this, &IdentityForm::onExportClicked);
    connect(bodyUI->deleteButton, &QPushButton::clicked, this, &IdentityForm::onDeleteClicked);
    connect(bodyUI->importButton, &QPushButton::clicked, this, &IdentityForm::onImportClicked);

    connect(Core::getInstance(), &Core::usernameSet, this, [=](const QString& val) { bodyUI->userName->setText(val); });
    connect(Core::getInstance(), &Core::statusMessageSet, this, [=](const QString& val) { bodyUI->statusMessage->setText(val); });
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
    toxId->setCursorPosition(0);
}

void IdentityForm::onUserNameEdited()
{
    Core::getInstance()->setUsername(bodyUI->userName->text());
}

void IdentityForm::onStatusMessageEdited()
{
    Core::getInstance()->setStatusMessage(bodyUI->statusMessage->text());
}

void IdentityForm::present()
{
    toxId->setText(Core::getInstance()->getSelfId().toString());
    toxId->setCursorPosition(0);
    bodyUI->profiles->clear();
    for (QString profile : Widget::searchProfiles())
        bodyUI->profiles->addItem(profile);
    QString current = Settings::getInstance().getCurrentProfile();
    if (current != "")
        bodyUI->profiles->setCurrentText(current);

    bodyUI->userName->setText(Core::getInstance()->getUsername());
    bodyUI->statusMessage->setText(Core::getInstance()->getStatusMessage());
}

void IdentityForm::setToxId(const QString& id)
{
    toxId->setText(id);
    toxId->setCursorPosition(0);
}

void IdentityForm::onLoadClicked()
{
    if (bodyUI->profiles->currentText() != Settings::getInstance().getCurrentProfile())
    {
        if (Core::getInstance()->anyActiveCalls())
            QMessageBox::warning(this, tr("Call active", "popup title"),
                tr("You can't switch profiles while a call is active!", "popup text"));
        else
            emit Widget::getInstance()->changeProfile(bodyUI->profiles->currentText());
            // I think by directly calling the function, I may have been causing thread issues
    }
}

void IdentityForm::onRenameClicked()
{
    QString cur = bodyUI->profiles->currentText();
    QString title = tr("Rename \"%1\"", "renaming a profile").arg(cur);
    QString name = QInputDialog::getText(this, title, title+":");
    if (name != "")
    {
        name = Core::sanitize(name);
        QDir dir(Settings::getSettingsDirPath());
        QFile::rename(dir.filePath(cur+Core::TOX_EXT), dir.filePath(name+Core::TOX_EXT));
        bodyUI->profiles->setItemText(bodyUI->profiles->currentIndex(), name);
        Settings::getInstance().setCurrentProfile(name);
    }
}

void IdentityForm::onExportClicked()
{
    QString current = bodyUI->profiles->currentText() + Core::TOX_EXT;
    QString path = QFileDialog::getSaveFileName(this, tr("Export profile", "save dialog title"),
                    QDir::home().filePath(current), 
                    tr("Tox save file (*.tox)", "save dialog filter"));
    if (!path.isEmpty())
        QFile::copy(QDir(Settings::getSettingsDirPath()).filePath(current), path);
}

void IdentityForm::onDeleteClicked()
{
    if (Settings::getInstance().getCurrentProfile() == bodyUI->profiles->currentText())
    {
        QMessageBox::warning(this, tr("Profile currently loaded","current profile deletion warning title"), tr("This profile is currently in use. Please load a different profile before deleting this one.","current profile deletion warning text"));
    }
    else
    {        
        QMessageBox::StandardButton resp = QMessageBox::question(this,
            tr("Deletion imminent!","deletion confirmation title"), tr("Are you sure you want to delete this profile?","deletion confirmation text"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (resp == QMessageBox::Yes)
        {
            QFile::remove(QDir(Settings::getSettingsDirPath()).filePath(bodyUI->profiles->currentText()+Core::TOX_EXT));
            bodyUI->profiles->removeItem(bodyUI->profiles->currentIndex());
            bodyUI->profiles->setCurrentText(Settings::getInstance().getCurrentProfile());
        }
    }
}

void IdentityForm::onImportClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Import profile", "import dialog title"), QDir::homePath(), tr("Tox save file (*.tox)", "import dialog filter"));
    if (path.isEmpty())
        return;

    QFileInfo info(path);

    if (info.suffix() != "tox")
    {
        QMessageBox::warning(this, tr("Ignoring non-Tox file", "popup title"), tr("Warning: you've chosen a file that is not a Tox save file; ignoring.", "popup text"));
        return;
    }

    QString profile = info.completeBaseName();
    QString profilePath = QDir(Settings::getSettingsDirPath()).filePath(profile + Core::TOX_EXT);
    QFile::copy(path, profilePath);
    bodyUI->profiles->addItem(profile);
}
