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

#include "generalform.h"
#include "widget/form/settingswidget.h"
#include "widget/widget.h"
#include "misc/settings.h"
#include "misc/smileypack.h"

GeneralForm::GeneralForm()
{
    icon.setPixmap(QPixmap(":/img/settings/general.png").scaledToHeight(headLayout.sizeHint().height(), Qt::SmoothTransformation));
    label.setText(tr("General Settings"));
    
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
    
    headLayout.addWidget(&label);
    layout.addWidget(&enableIPv6);
    layout.addWidget(&useTranslations);
    layout.addWidget(&makeToxPortable);
    layout.addWidget(&smileyPackLabel);
    layout.addWidget(&smileyPackBrowser);
    layout.addStretch();
    
    connect(&enableIPv6, SIGNAL(stateChanged(int)), this, SLOT(onEnableIPv6Updated()));
    connect(&useTranslations, SIGNAL(stateChanged(int)), this, SLOT(onUseTranslationUpdated()));
    connect(&makeToxPortable, SIGNAL(stateChanged(int)), this, SLOT(onMakeToxPortableUpdated()));
    connect(&smileyPackBrowser, SIGNAL(currentIndexChanged(int)), this, SLOT(onSmileyBrowserIndexChanged(int)));
}

GeneralForm::~GeneralForm()
{
}

void GeneralForm::onEnableIPv6Updated()
{
    Settings::getInstance().setEnableIPv6(enableIPv6.isChecked());
}

void GeneralForm::onUseTranslationUpdated()
{
    Settings::getInstance().setUseTranslations(useTranslations.isChecked());
}

void GeneralForm::onMakeToxPortableUpdated()
{
    Settings::getInstance().setMakeToxPortable(makeToxPortable.isChecked());
}

void GeneralForm::onSmileyBrowserIndexChanged(int index)
{
    QString filename = smileyPackBrowser.itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
}
