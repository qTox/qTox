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
#include <QFont>
#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QDir>

SettingsForm::SettingsForm()
    : QObject()
{
    main = new QWidget(), head = new QWidget();
    QFont bold, small;
    bold.setBold(true);
    small.setPixelSize(13);
    headLabel.setText(tr("User Settings","\"Headline\" of the window"));
    headLabel.setFont(bold);

    nameLabel.setText(tr("Name","Username/nick"));
    statusTextLabel.setText(tr("Status","Status message"));
    idLabel.setText("Tox ID " + tr("(click here to copy)", "Click on this text to copy TID to clipboard"));
    id.setFont(small);
    id.setTextInteractionFlags(Qt::TextSelectableByMouse);
    id.setReadOnly(true);
    id.setFrameStyle(QFrame::NoFrame);
    id.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    id.setFixedHeight(id.document()->size().height());

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
    layout.addWidget(&nameLabel);
    layout.addWidget(&name);
    layout.addWidget(&statusTextLabel);
    layout.addWidget(&statusText);
    layout.addWidget(&idLabel);
    layout.addWidget(&id);
    layout.addWidget(&videoTest);
    layout.addWidget(&enableIPv6);
    layout.addWidget(&useTranslations);
    layout.addWidget(&makeToxPortable);
    layout.addWidget(&smileyPackLabel);
    layout.addWidget(&smileyPackBrowser);
    layout.addStretch();

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

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

void SettingsForm::setFriendAddress(const QString& friendAddress)
{
    id.setText(friendAddress);
}

void SettingsForm::show(Ui::Widget &ui)
{
    name.setText(ui.nameLabel->text());
    statusText.setText(ui.statusLabel->text());
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
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
    id.selectAll();;
    QApplication::clipboard()->setText(id.toPlainText());
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
