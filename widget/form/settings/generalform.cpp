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
#include <QMessageBox>

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
    
    bodyUI->cbUDPDisabled->setChecked(Settings::getInstance().getForceTCP());
    bodyUI->proxyAddr->setText(Settings::getInstance().getProxyAddr());
    int port = Settings::getInstance().getProxyPort();
    if (port != -1)
        bodyUI->proxyPort->setText(QString::number(port));
    
    connect(bodyUI->cbEnableIPv6, &QCheckBox::stateChanged, this, &GeneralForm::onEnableIPv6Updated);
    connect(bodyUI->cbUseTranslations, &QCheckBox::stateChanged, this, &GeneralForm::onUseTranslationUpdated);
    connect(bodyUI->cbMakeToxPortable, &QCheckBox::stateChanged, this, &GeneralForm::onMakeToxPortableUpdated);
    connect(bodyUI->smileyPackBrowser, SIGNAL(currentIndexChanged(int)), this, SLOT(onSmileyBrowserIndexChanged(int)));
    // new syntax can't handle overloaded signals... (at least not in a pretty way)
    connect(bodyUI->cbUDPDisabled, &QCheckBox::stateChanged, this, &GeneralForm::onUDPUpdated);
    connect(bodyUI->proxyAddr, &QLineEdit::editingFinished, this, &GeneralForm::onProxyAddrEdited);
    connect(bodyUI->proxyPort, &QLineEdit::editingFinished, this, &GeneralForm::onProxyPortEdited);
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

void GeneralForm::onUDPUpdated()
{
    Settings::getInstance().setForceTCP(bodyUI->cbUDPDisabled->isChecked());
}

void GeneralForm::onProxyAddrEdited()
{
    Settings::getInstance().setProxyAddr(bodyUI->proxyAddr->text());
}

void GeneralForm::onProxyPortEdited()
{
    QString text = bodyUI->proxyPort->text();
    if (text != "")
    {
        int port = text.toInt();
        if (port < 1)
            QMessageBox::warning(bodyUI->proxyPort, tr("Bad port", "title of bad port popup"), tr("The port you entered is invalid; please enter another.", "text of bad port popup"));
        else
            Settings::getInstance().setProxyPort(port);
    }
    else
        Settings::getInstance().setProxyPort(-1);
}
