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
#include "src/widget/form/settingswidget.h"
#include "src/widget/widget.h"
#include "src/misc/settings.h"
#include "src/misc/smileypack.h"
#include <QMessageBox>
#include <QStyleFactory>

GeneralForm::GeneralForm(SettingsWidget *myParent) :
    GenericForm(tr("General Settings"), QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::GeneralSettings;
    bodyUI->setupUi(this);
    
    parent = myParent;    

    bodyUI->cbEnableIPv6->setChecked(Settings::getInstance().getEnableIPv6());
    bodyUI->cbUseTranslations->setChecked(Settings::getInstance().getUseTranslations());
    bodyUI->cbMakeToxPortable->setChecked(Settings::getInstance().getMakeToxPortable());
    bodyUI->startInTray->setChecked(Settings::getInstance().getAutostartInTray());
    bodyUI->statusChangesCheckbox->setChecked(Settings::getInstance().getStatusChangeNotificationEnabled());

    for (auto entry : SmileyPack::listSmileyPacks())
    {
        bodyUI->smileyPackBrowser->addItem(entry.first, entry.second);
    }
    bodyUI->smileyPackBrowser->setCurrentIndex(bodyUI->smileyPackBrowser->findData(Settings::getInstance().getSmileyPack()));
    reloadSmiles();

    bodyUI->styleBrowser->addItems(QStyleFactory::keys());
    bodyUI->styleBrowser->addItem("None");
        
    if(QStyleFactory::keys().contains(Settings::getInstance().getStyle()))
        bodyUI->styleBrowser->setCurrentText(Settings::getInstance().getStyle());
    else
        bodyUI->styleBrowser->setCurrentText("None");
    
    bodyUI->autoAwaySpinBox->setValue(Settings::getInstance().getAutoAwayTime());
    
    bodyUI->cbUDPDisabled->setChecked(Settings::getInstance().getForceTCP());
    bodyUI->proxyAddr->setText(Settings::getInstance().getProxyAddr());
    int port = Settings::getInstance().getProxyPort();
    if (port != -1)
        bodyUI->proxyPort->setValue(port);

    bodyUI->cbUseProxy->setChecked(Settings::getInstance().getUseProxy());
    onUseProxyUpdated();

    connect(bodyUI->cbEnableIPv6, &QCheckBox::stateChanged, this, &GeneralForm::onEnableIPv6Updated);
    connect(bodyUI->cbUseTranslations, &QCheckBox::stateChanged, this, &GeneralForm::onUseTranslationUpdated);
    connect(bodyUI->cbMakeToxPortable, &QCheckBox::stateChanged, this, &GeneralForm::onMakeToxPortableUpdated);
    connect(bodyUI->startInTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetAutostartInTray);
    connect(bodyUI->statusChangesCheckbox, &QCheckBox::stateChanged, this, &GeneralForm::onSetStatusChange);
    connect(bodyUI->smileyPackBrowser, SIGNAL(currentIndexChanged(int)), this, SLOT(onSmileyBrowserIndexChanged(int)));
    // new syntax can't handle overloaded signals... (at least not in a pretty way)
    connect(bodyUI->cbUDPDisabled, &QCheckBox::stateChanged, this, &GeneralForm::onUDPUpdated);
    connect(bodyUI->proxyAddr, &QLineEdit::editingFinished, this, &GeneralForm::onProxyAddrEdited);
    connect(bodyUI->proxyPort, SIGNAL(valueChanged(int)), this, SLOT(onProxyPortEdited(int)));
    connect(bodyUI->cbUseProxy, &QCheckBox::stateChanged, this, &GeneralForm::onUseProxyUpdated);
    connect(bodyUI->styleBrowser, SIGNAL(currentTextChanged(QString)), this, SLOT(onStyleSelected(QString)));
    connect(bodyUI->autoAwaySpinBox, SIGNAL(editingFinished()), this, SLOT(onAutoAwayChanged()));
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

void GeneralForm::onSetAutostartInTray()
{
    Settings::getInstance().setAutostartInTray(bodyUI->startInTray->isChecked());
}

void GeneralForm::onStyleSelected(QString style)
{
    Settings::getInstance().setStyle(style);
    this->setStyle(QStyleFactory::create(style));
    parent->setStyle(style);
}

void GeneralForm::onAutoAwayChanged()
{
    int minutes = bodyUI->autoAwaySpinBox->value();
    Settings::getInstance().setAutoAwayTime(minutes);
    Widget::getInstance()->setIdleTimer(minutes);
}

void GeneralForm::onSetStatusChange()
{
    Settings::getInstance().setStatusChangeNotificationEnabled(bodyUI->statusChangesCheckbox->isChecked());
}

void GeneralForm::onSmileyBrowserIndexChanged(int index)
{
    QString filename = bodyUI->smileyPackBrowser->itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
    reloadSmiles();
}

void GeneralForm::onUDPUpdated()
{
    Settings::getInstance().setForceTCP(bodyUI->cbUDPDisabled->isChecked());
}

void GeneralForm::onProxyAddrEdited()
{
    Settings::getInstance().setProxyAddr(bodyUI->proxyAddr->text());
}

void GeneralForm::onProxyPortEdited(int port)
{
    if (port > 0)
    {
        Settings::getInstance().setProxyPort(port);
    } else {
        Settings::getInstance().setProxyPort(-1);
    }
}

void GeneralForm::onUseProxyUpdated()
{
    bool state = bodyUI->cbUseProxy->isChecked();

    bodyUI->proxyAddr->setEnabled(state);
    bodyUI->proxyPort->setEnabled(state);
    Settings::getInstance().setUseProxy(state);
}

void GeneralForm::reloadSmiles()
{
    QList<QStringList> emoticons = SmileyPack::getInstance().getEmoticons();
    QStringList smiles;
    smiles << ":)" << ";)" << ":p" << ":O" << ":["; //just in case...

    for(int i = 0; i < emoticons.size(); i++)  
        smiles.push_front(emoticons.at(i).first());
        
    int pixSize = 30;
    bodyUI->smile1->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[0]).pixmap(pixSize, pixSize));
    bodyUI->smile2->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[1]).pixmap(pixSize, pixSize));
    bodyUI->smile3->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[2]).pixmap(pixSize, pixSize));
    bodyUI->smile4->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[3]).pixmap(pixSize, pixSize));
    bodyUI->smile5->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[4]).pixmap(pixSize, pixSize));
    
    bodyUI->smile1->setToolTip(smiles[0]);    
    bodyUI->smile2->setToolTip(smiles[1]);
    bodyUI->smile3->setToolTip(smiles[2]);
    bodyUI->smile4->setToolTip(smiles[3]);
    bodyUI->smile5->setToolTip(smiles[4]);
}
