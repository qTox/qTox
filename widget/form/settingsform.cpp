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

#include "settingsform.h"
#include "widget/widget.h"
#include "settings.h"
#include "smileypack.h"
#include "ui_mainwindow.h"
#include <QFont>
#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include "core.h"

SettingsForm::SettingsForm()
    : QObject()
{
    main = new QWidget(), head = new QWidget();
    hboxcont1 = new QWidget(), hboxcont2 = new QWidget();
    QFont bold, small;
    bold.setBold(true);
    small.setPixelSize(13);
    small.setKerning(false);
    headLabel.setText(tr("User Settings","\"Headline\" of the window"));
    headLabel.setFont(bold);
    
    idLabel.setText("Tox ID " + tr("(click here to copy)", "Click on this text to copy TID to clipboard"));
    id.setFont(small);
    id.setTextInteractionFlags(Qt::TextSelectableByMouse);
    id.setReadOnly(true);
    id.setFrameStyle(QFrame::NoFrame);
    id.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    id.setFixedHeight(id.document()->size().height()*2);
    
    profilesLabel.setText(tr("Available profiles:", "Labels the profile selection box"));
    loadConf.setText(tr("Load profile", "button to load selected profile"));
    exportConf.setText(tr("Export profile", "button to save selected profile elsewhere"));
    delConf.setText(tr("Delete profile", "button to delete selected profile from disk"));
    delConf.setToolTip(tr("This is useful to remain safe on public computers", "describes the delete profile button"));
    //delConf.setWhatsThis(tr("This is useful to remain safe on public computers", "describes the delete profile button"));
    importConf.setText(tr("Import profile", "button to locate a profile"));

    videoTest.setText(tr("Test video","Text on a button to test the video/webcam"));
    enableIPv6.setText(tr("Enable IPv6 (recommended)","Text on a checkbox to enable IPv6"));
    enableIPv6.setChecked(Settings::getInstance().getEnableIPv6());
    useTranslations.setText(tr("Use translations","Text on a checkbox to enable translations"));
    useTranslations.setChecked(Settings::getInstance().getUseTranslations());
    makeToxPortable.setText(tr("Make Tox portable","Text on a checkbox to make qTox a portable application"));
    makeToxPortable.setChecked(Settings::getInstance().getMakeToxPortable());
    makeToxPortable.setToolTip(tr("Save settings to the working directory instead of the usual conf dir","describes makeToxPortable checkbox"));

    smileyPackLabel.setText(tr("Smiley Pack", "Text on smiley pack label"));
    for (auto entry : SmileyPack::listSmileyPacks())
        smileyPackBrowser.addItem(entry.first, entry.second);
    smileyPackBrowser.setCurrentIndex(smileyPackBrowser.findData(Settings::getInstance().getSmileyPack()));

    main->setLayout(&layout);
    layout.addWidget(&idLabel);
    layout.addWidget(&id);
    cbox.addWidget(&profilesLabel);
    cbox.addWidget(&profiles);
    hboxcont1->setLayout(&cbox);
    layout.addWidget(hboxcont1);
    buttons.addWidget(&loadConf);
    buttons.addWidget(&exportConf);
    buttons.addWidget(&delConf);
    hboxcont2->setLayout(&buttons);
    layout.addWidget(hboxcont2);
    layout.addWidget(&importConf);
    layout.addWidget(&videoTest);
    layout.addWidget(&enableIPv6);
    layout.addWidget(&useTranslations);
    layout.addWidget(&makeToxPortable);
    layout.addWidget(&smileyPackLabel);
    layout.addWidget(&smileyPackBrowser);
    layout.addStretch();

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(&loadConf, SIGNAL(clicked()), this, SLOT(onLoadClicked()));
    connect(&exportConf, SIGNAL(clicked()), this, SLOT(onExportClicked()));
    connect(&delConf, SIGNAL(clicked()), this, SLOT(onDeleteClicked()));
    connect(&importConf, SIGNAL(clicked()), this, SLOT(onImportClicked()));
    connect(&videoTest, SIGNAL(clicked()), this, SLOT(onTestVideoClicked()));
    connect(&enableIPv6, SIGNAL(stateChanged(int)), this, SLOT(onEnableIPv6Updated()));
    connect(&useTranslations, SIGNAL(stateChanged(int)), this, SLOT(onUseTranslationUpdated()));
    connect(&makeToxPortable, SIGNAL(stateChanged(int)), this, SLOT(onMakeToxPortableUpdated()));
    connect(&idLabel, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(&smileyPackBrowser, SIGNAL(currentIndexChanged(int)), this, SLOT(onSmileyBrowserIndexChanged(int)));
}

SettingsForm::~SettingsForm()
{
}

QList<QString> SettingsForm::searchProfiles()
{
    QList<QString> out;
    QDir dir(Settings::getSettingsDirPath());
	dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	dir.setNameFilters(QStringList("*.tox"));
	for(QFileInfo file : dir.entryInfoList())
	{
		out += file.completeBaseName();
	}
	return out;
}

QString SettingsForm::getSelectedSavePath()
{
    return Settings::getSettingsDirPath() + QDir::separator() + profiles.currentText() + Core::getInstance()->TOX_EXT;
}

void SettingsForm::setFriendAddress(const QString& friendAddress)
{
    QString txt{friendAddress};
    txt.insert(38,'\n');
    id.setText(txt);
}

void SettingsForm::show(Ui::MainWindow &ui)
{
    profiles.clear();
    for (QString profile : searchProfiles())
    {
        profiles.addItem(profile);
    }
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

void SettingsForm::onLoadClicked()
{
    Core::getInstance()->switchConfiguration(profiles.currentText());
}

void SettingsForm::onExportClicked()
{
    QString current = getSelectedSavePath();
    QString path = QFileDialog::getSaveFileName(0, tr("Export profile", "save dialog title"), QDir::homePath() + QDir::separator() + profiles.currentText() + Core::getInstance()->TOX_EXT, tr("Tox save file (*.tox)", "save dialog filter"));
    QFile::copy(getSelectedSavePath(), path);
}

void SettingsForm::onDeleteClicked()
{
    if (Settings::getInstance().getCurrentProfile() == profiles.currentText())
    {
        QMessageBox::warning(main, tr("Profile currently loaded","current profile deletion warning title"), tr("This profile is currently in use. Please load a different profile before deleting this one.","current profile deletion warning text"));
    }
    else
    {        
        QMessageBox::StandardButton resp = QMessageBox::question(main,
            tr("Deletion imminent!","deletion confirmation title"), tr("Are you sure you want to delete this profile?","deletion confirmation text"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (resp == QMessageBox::Yes)
        {
            QFile::remove(getSelectedSavePath());
            profiles.removeItem(profiles.currentIndex());
        }
    }
}

void SettingsForm::onImportClicked()
{
    QString path = QFileDialog::getOpenFileName(0, tr("Import profile", "import dialog title"), QDir::homePath(), tr("Tox save file (*.tox)", "import dialog filter"));
    QFileInfo info(path);
    QString profile = info.completeBaseName();
    QString profilePath = Settings::getSettingsDirPath() + profile + Core::getInstance()->TOX_EXT;
    Settings::getInstance().setCurrentProfile(profile);
    QFile::copy(path, profilePath);
    profiles.addItem(profile);
    Core::getInstance()->switchConfiguration(profile);
}

void SettingsForm::onTestVideoClicked()
{
     Widget::getInstance()->showTestCamview();
}

void SettingsForm::onEnableIPv6Updated()
{
    Settings::getInstance().setEnableIPv6(enableIPv6.isChecked());
}

void SettingsForm::copyIdClicked()
{
    id.selectAll();
    QString txt = id.toPlainText();
    txt.replace('\n',"");
    QApplication::clipboard()->setText(txt);
}

void SettingsForm::onUseTranslationUpdated()
{
    Settings::getInstance().setUseTranslations(useTranslations.isChecked());
}

void SettingsForm::onMakeToxPortableUpdated()
{
    Settings::getInstance().setMakeToxPortable(makeToxPortable.isChecked());
}

void SettingsForm::onSmileyBrowserIndexChanged(int index)
{
    QString filename = smileyPackBrowser.itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
}
