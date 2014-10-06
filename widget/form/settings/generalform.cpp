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

#include "ui_generalsettings.h"
#include "generalform.h"
#include "widget/form/settingswidget.h"
#include "widget/widget.h"
#include "misc/settings.h"
#include "misc/smileypack.h"

GeneralForm::GeneralForm() :
    GenericForm(tr("General Settings"), QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::GeneralSettings;
    bodyUI->setupUi(this);

    bodyUI->cbEnableIPv6->setChecked(Settings::getInstance().getEnableIPv6());
    bodyUI->cbUseTranslations->setChecked(Settings::getInstance().getUseTranslations());
    bodyUI->cbMakeToxPortable->setChecked(Settings::getInstance().getMakeToxPortable());

    for (auto entry : SmileyPack::listSmileyPacks())
    {
        bodyUI->smileyPackBrowser->addItem(entry.first, entry.second);
    }
    bodyUI->smileyPackBrowser->setCurrentIndex(bodyUI->smileyPackBrowser->findData(Settings::getInstance().getSmileyPack()));
    
    connect(bodyUI->cbEnableIPv6, SIGNAL(stateChanged(int)), this, SLOT(onEnableIPv6Updated()));
    connect(bodyUI->cbUseTranslations, SIGNAL(stateChanged(int)), this, SLOT(onUseTranslationUpdated()));
    connect(bodyUI->cbMakeToxPortable, SIGNAL(stateChanged(int)), this, SLOT(onMakeToxPortableUpdated()));
    connect(bodyUI->smileyPackBrowser, SIGNAL(currentIndexChanged(int)), this, SLOT(onSmileyBrowserIndexChanged(int)));
}

GeneralForm::~GeneralForm()
{
    delete bodyUI;
}

void GeneralForm::onEnableIPv6Updated()
{
    Settings::getInstance().setEnableIPv6(bodyUI->cbEnableIPv6->isChecked());
}

void GeneralForm::onUseTranslationUpdated()
{
    Settings::getInstance().setUseTranslations(bodyUI->cbUseTranslations->isChecked());
}

void GeneralForm::onMakeToxPortableUpdated()
{
    Settings::getInstance().setMakeToxPortable(bodyUI->cbMakeToxPortable->isChecked());
}

void GeneralForm::onSmileyBrowserIndexChanged(int index)
{
    QString filename = bodyUI->smileyPackBrowser->itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
}
