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
#include "src/widget/form/checkcontinue.h"
#include "src/widget/widget.h"
#include "src/historykeeper.h"
#include "src/misc/style.h"
#include <QLabel>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QFileDialog>

IdentityForm::IdentityForm() :
    GenericForm(tr("Identity"), QPixmap(":/img/settings/identity.png"))
{
    bodyUI = new Ui::IdentitySettings;
    bodyUI->setupUi(this);
    core = Core::getInstance();

    // tox
    toxId = new ClickableTE();
    toxId->setReadOnly(true);
    toxId->setFrame(false);
    toxId->setFont(Style::getFont(Style::Small));
    
    bodyUI->toxGroup->layout()->addWidget(toxId);

    timer.setInterval(1000);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [=]() {bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text().replace(" ✔", ""));});
    
    connect(bodyUI->toxIdLabel, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(toxId, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(core, &Core::idSet, this, &IdentityForm::setToxId);
    connect(bodyUI->userName, SIGNAL(editingFinished()), this, SLOT(onUserNameEdited()));
    connect(bodyUI->statusMessage, SIGNAL(editingFinished()), this, SLOT(onStatusMessageEdited()));
    connect(bodyUI->loadButton, &QPushButton::clicked, this, &IdentityForm::onLoadClicked);
    connect(bodyUI->renameButton, &QPushButton::clicked, this, &IdentityForm::onRenameClicked);
    connect(bodyUI->exportButton, &QPushButton::clicked, this, &IdentityForm::onExportClicked);
    connect(bodyUI->deleteButton, &QPushButton::clicked, this, &IdentityForm::onDeleteClicked);
    connect(bodyUI->importButton, &QPushButton::clicked, this, &IdentityForm::onImportClicked);
    connect(bodyUI->newButton, &QPushButton::clicked, this, &IdentityForm::onNewClicked);

    connect(core, &Core::avStart, this, &IdentityForm::disableSwitching);
    connect(core, &Core::avStarting, this, &IdentityForm::disableSwitching);
    connect(core, &Core::avInvite, this, &IdentityForm::disableSwitching);
    connect(core, &Core::avRinging, this, &IdentityForm::disableSwitching);
    connect(core, &Core::avCancel, this, &IdentityForm::enableSwitching);
    connect(core, &Core::avEnd, this, &IdentityForm::enableSwitching);
    connect(core, &Core::avEnding, this, &IdentityForm::enableSwitching);
    connect(core, &Core::avPeerTimeout, this, &IdentityForm::enableSwitching);
    connect(core, &Core::avRequestTimeout, this, &IdentityForm::enableSwitching);

    connect(core, &Core::usernameSet, this, [=](const QString& val) { bodyUI->userName->setText(val); });
    connect(core, &Core::statusMessageSet, this, [=](const QString& val) { bodyUI->statusMessage->setText(val); });
}

IdentityForm::~IdentityForm()
{
    delete bodyUI;
}

void IdentityForm::copyIdClicked()
{
    toxId->selectAll();
    QString txt = toxId->text();
    txt.replace('\n',"");
    QApplication::clipboard()->setText(txt);
    toxId->setCursorPosition(0);

    bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text() + " ✔");
    timer.start();
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
    do
    {
        QString name = QInputDialog::getText(this, title, title+":");
        if (name.isEmpty()) break;
        name = Core::sanitize(name);
        QDir dir(Settings::getSettingsDirPath());
        QString file = dir.filePath(name+Core::TOX_EXT);
        if (!QFile::exists(file) || checkContinue(tr("Profile already exists", "rename confirm title"),
                tr("A profile named \"%1\" already exists. Do you want to erase it?", "rename confirm text").arg(cur)), this)
        {
            QFile::rename(dir.filePath(cur+Core::TOX_EXT), file);
            bodyUI->profiles->setItemText(bodyUI->profiles->currentIndex(), name);
            HistoryKeeper::renameHistory(cur, name);
            Settings::getInstance().setCurrentProfile(name);
            break;
        }
    } while (true);
}

void IdentityForm::onExportClicked()
{
    QString current = bodyUI->profiles->currentText() + Core::TOX_EXT;
    QString path = QFileDialog::getSaveFileName(this, tr("Export profile", "save dialog title"),
                    QDir::home().filePath(current), 
                    tr("Tox save file (*.tox)", "save dialog filter"));
    if (!path.isEmpty())
    {
        bool success;
        if (QFile::exists(path))
        {
            // should we popup a warning?
            // if (!checkContinue(tr("Overwriting a file"), tr("Are you sure you want to overwrite %1?").arg(path)), this)
            //     return;
            success = QFile::remove(path);
            if (!success)
            {
                QMessageBox::warning(this, tr("Failed to remove file"), tr("The file you chose to overwrite could not be removed first."));
                return;
            }
        }
        success = QFile::copy(QDir(Settings::getSettingsDirPath()).filePath(current), path);
        if (!success)
            QMessageBox::warning(this, tr("Failed to copy file"), tr("The file you chose could not be written to."));
    }
}

void IdentityForm::onDeleteClicked()
{
    if (Settings::getInstance().getCurrentProfile() == bodyUI->profiles->currentText())
    {
        QMessageBox::warning(this, tr("Profile currently loaded","current profile deletion warning title"), tr("This profile is currently in use. Please load a different profile before deleting this one.","current profile deletion warning text"));
    }
    else
    {        
        if (checkContinue(tr("Deletion imminent!","deletion confirmation title"),
                          tr("Are you sure you want to delete this profile?","deletion confirmation text"), this))
        {
            QString profile = bodyUI->profiles->currentText();
            QDir dir(Settings::getSettingsDirPath());

            QFile::remove(dir.filePath(profile + Core::TOX_EXT));
            QFile::remove(dir.filePath(profile + ".ini"));
            QFile::remove(HistoryKeeper::getHistoryPath(profile, 0));
            QFile::remove(HistoryKeeper::getHistoryPath(profile, 1));

            bodyUI->profiles->removeItem(bodyUI->profiles->currentIndex());
            bodyUI->profiles->setCurrentText(Settings::getInstance().getCurrentProfile());
        }
    }
}

void IdentityForm::onImportClicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Import profile", "import dialog title"),
                                                QDir::homePath(),
                                                tr("Tox save file (*.tox)", "import dialog filter"));
    if (path.isEmpty())
        return;

    QFileInfo info(path);
    QString profile = info.completeBaseName();

    if (info.suffix() != "tox")
    {
        QMessageBox::warning(this,
                             tr("Ignoring non-Tox file", "popup title"),
                             tr("Warning: you've chosen a file that is not a Tox save file; ignoring.", "popup text"));
        return;
    }

    QString profilePath = QDir(Settings::getSettingsDirPath()).filePath(profile + Core::TOX_EXT);

    if (QFileInfo(profilePath).exists() && !checkContinue(tr("Profile already exists", "import confirm title"),
            tr("A profile named \"%1\" already exists. Do you want to erase it?", "import confirm text").arg(profile), this))
        return;

    QFile::copy(path, profilePath);
    bodyUI->profiles->addItem(profile);
}

void IdentityForm::onNewClicked()
{
    emit Widget::getInstance()->changeProfile(QString());
}

void IdentityForm::disableSwitching()
{
    bodyUI->loadButton->setEnabled(false);
    bodyUI->newButton->setEnabled(false);
}

void IdentityForm::enableSwitching()
{
    if (!core->anyActiveCalls())
    {
        bodyUI->loadButton->setEnabled(true);
        bodyUI->newButton->setEnabled(true);
    }
}
