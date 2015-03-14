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
#include "src/core.h"
#include "src/misc/style.h"
#include <QMessageBox>
#include <QStyleFactory>
#include <QTime>
#include <QFileDialog>
#include <QStandardPaths>

#include "src/autoupdate.h"

static QStringList locales = {"bg", "de", "en", "es", "fr", "hr", "hu", "it", "lt", "mannol", "nl", "pirate", "pl", "pt", "ru", "sl", "fi", "sv", "uk", "zh"};
static QStringList langs = {"Български", "Deutsch", "English", "Español", "Français", "Hrvatski", "Magyar", "Italiano", "Lietuvių", "mannol", "Nederlands", "Pirate", "Polski", "Português", "Русский", "Slovenščina", "Suomi", "Svenska", "Українська", "简体中文"};

static QStringList timeFormats = {"hh:mm AP", "hh:mm", "hh:mm:ss AP", "hh:mm:ss"};

GeneralForm::GeneralForm(SettingsWidget *myParent) :
    GenericForm(tr("General"), QPixmap(":/img/settings/general.png"))
{
    parent = myParent;

    bodyUI = new Ui::GeneralSettings;
    bodyUI->setupUi(this);

    bodyUI->checkUpdates->setVisible(AUTOUPDATE_ENABLED);
    bodyUI->checkUpdates->setChecked(Settings::getInstance().getCheckUpdates());

    bodyUI->cbEnableIPv6->setChecked(Settings::getInstance().getEnableIPv6());
    for (int i = 0; i < langs.size(); i++)
        bodyUI->transComboBox->insertItem(i, langs[i]);
    bodyUI->transComboBox->setCurrentIndex(locales.indexOf(Settings::getInstance().getTranslation()));
    bodyUI->cbAutorun->setChecked(Settings::getInstance().getAutorun());
#if defined(__APPLE__) && defined(__MACH__)
    bodyUI->cbAutorun->setEnabled(false);
#endif

    bool showSystemTray = Settings::getInstance().getShowSystemTray();

    bodyUI->showSystemTray->setChecked(showSystemTray);
    bodyUI->startInTray->setChecked(Settings::getInstance().getAutostartInTray());
    bodyUI->startInTray->setEnabled(showSystemTray);
    bodyUI->closeToTray->setChecked(Settings::getInstance().getCloseToTray());
    bodyUI->closeToTray->setEnabled(showSystemTray);
    bodyUI->minimizeToTray->setChecked(Settings::getInstance().getMinimizeToTray());
    bodyUI->minimizeToTray->setEnabled(showSystemTray);
    bodyUI->lightTrayIcon->setChecked(Settings::getInstance().getLightTrayIcon());
    bodyUI->lightTrayIcon->setEnabled(showSystemTray);
    bodyUI->statusChanges->setChecked(Settings::getInstance().getStatusChangeNotificationEnabled());
    bodyUI->useEmoticons->setChecked(Settings::getInstance().getUseEmoticons());
    bodyUI->autoacceptFiles->setChecked(Settings::getInstance().getAutoSaveEnabled());
    bodyUI->autoSaveFilesDir->setText(Settings::getInstance().getGlobalAutoAcceptDir());
    bodyUI->showWindow->setChecked(Settings::getInstance().getShowWindow());
    bodyUI->showInFront->setChecked(Settings::getInstance().getShowInFront());
    bodyUI->groupAlwaysNotify->setChecked(Settings::getInstance().getGroupAlwaysNotify());
    bodyUI->cbFauxOfflineMessaging->setChecked(Settings::getInstance().getFauxOfflineMessaging());
    bodyUI->cbCompactLayout->setChecked(Settings::getInstance().getCompactLayout());

    for (auto entry : SmileyPack::listSmileyPacks())
    {
        bodyUI->smileyPackBrowser->addItem(entry.first, entry.second);
    }
    bodyUI->smileyPackBrowser->setCurrentIndex(bodyUI->smileyPackBrowser->findData(Settings::getInstance().getSmileyPack()));
    reloadSmiles();

    bodyUI->styleBrowser->addItem(tr("None"));
    bodyUI->styleBrowser->addItems(QStyleFactory::keys());

    if (QStyleFactory::keys().contains(Settings::getInstance().getStyle()))
        bodyUI->styleBrowser->setCurrentText(Settings::getInstance().getStyle());
    else
        bodyUI->styleBrowser->setCurrentText(tr("None"));

    for (QString color : Style::themeColorNames)
        bodyUI->themeColorCBox->addItem(color);
    bodyUI->themeColorCBox->setCurrentIndex(Settings::getInstance().getThemeColor());

    bodyUI->emoticonSize->setValue(Settings::getInstance().getEmojiFontPointSize());

    QStringList timestamps;
    timestamps << QString("%1 - %2").arg(timeFormats[0],QTime::currentTime().toString(timeFormats[0]))
               << QString("%1 - %2").arg(timeFormats[1],QTime::currentTime().toString(timeFormats[1]))
               << QString("%1 - %2").arg(timeFormats[2],QTime::currentTime().toString(timeFormats[2]))
               << QString("%1 - %2").arg(timeFormats[3],QTime::currentTime().toString(timeFormats[3]));
    bodyUI->timestamp->addItems(timestamps);

    bodyUI->timestamp->setCurrentText(QString("%1 - %2").arg(Settings::getInstance().getTimestampFormat(),
                                                             QTime::currentTime().toString(Settings::getInstance().getTimestampFormat()))
                                      ); //idiot proof enough?

    bodyUI->autoAwaySpinBox->setValue(Settings::getInstance().getAutoAwayTime());

    bodyUI->cbEnableUDP->setChecked(!Settings::getInstance().getForceTCP());
    bodyUI->proxyAddr->setText(Settings::getInstance().getProxyAddr());
    int port = Settings::getInstance().getProxyPort();
    if (port != -1)
        bodyUI->proxyPort->setValue(port);

    bodyUI->proxyType->setCurrentIndex(static_cast<int>(Settings::getInstance().getProxyType()));
    onUseProxyUpdated();

    //general
    connect(bodyUI->checkUpdates, &QCheckBox::stateChanged, this, &GeneralForm::onCheckUpdateChanged);
    connect(bodyUI->transComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTranslationUpdated()));
    connect(bodyUI->cbAutorun, &QCheckBox::stateChanged, this, &GeneralForm::onAutorunUpdated);
    connect(bodyUI->showSystemTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetShowSystemTray);
    connect(bodyUI->startInTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetAutostartInTray);
    connect(bodyUI->closeToTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetCloseToTray);
    connect(bodyUI->minimizeToTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetMinimizeToTray);
    connect(bodyUI->lightTrayIcon, &QCheckBox::stateChanged, this, &GeneralForm::onSetLightTrayIcon);
    connect(bodyUI->statusChanges, &QCheckBox::stateChanged, this, &GeneralForm::onSetStatusChange);
    connect(bodyUI->autoAwaySpinBox, SIGNAL(editingFinished()), this, SLOT(onAutoAwayChanged()));
    connect(bodyUI->showWindow, &QCheckBox::stateChanged, this, &GeneralForm::onShowWindowChanged);
    connect(bodyUI->showInFront, &QCheckBox::stateChanged, this, &GeneralForm::onSetShowInFront);
    connect(bodyUI->groupAlwaysNotify, &QCheckBox::stateChanged, this, &GeneralForm::onSetGroupAlwaysNotify);
    connect(bodyUI->autoacceptFiles, &QCheckBox::stateChanged, this, &GeneralForm::onAutoAcceptFileChange);
    if (bodyUI->autoacceptFiles->isChecked())
        connect(bodyUI->autoSaveFilesDir, SIGNAL(clicked()), this, SLOT(onAutoSaveDirChange()));
    //theme
    connect(bodyUI->useEmoticons, &QCheckBox::stateChanged, this, &GeneralForm::onUseEmoticonsChange);
    connect(bodyUI->smileyPackBrowser, SIGNAL(currentIndexChanged(int)), this, SLOT(onSmileyBrowserIndexChanged(int)));
    connect(bodyUI->styleBrowser, SIGNAL(currentTextChanged(QString)), this, SLOT(onStyleSelected(QString)));
    connect(bodyUI->themeColorCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onThemeColorChanged(int)));
    connect(bodyUI->emoticonSize, SIGNAL(editingFinished()), this, SLOT(onEmoticonSizeChanged()));
    connect(bodyUI->timestamp, SIGNAL(currentIndexChanged(int)), this, SLOT(onTimestampSelected(int)));
    //connection
    connect(bodyUI->cbEnableIPv6, &QCheckBox::stateChanged, this, &GeneralForm::onEnableIPv6Updated);
    connect(bodyUI->cbEnableUDP, &QCheckBox::stateChanged, this, &GeneralForm::onUDPUpdated);
    connect(bodyUI->proxyType, SIGNAL(currentIndexChanged(int)), this, SLOT(onUseProxyUpdated()));
    connect(bodyUI->proxyAddr, &QLineEdit::editingFinished, this, &GeneralForm::onProxyAddrEdited);
    connect(bodyUI->proxyPort, SIGNAL(valueChanged(int)), this, SLOT(onProxyPortEdited(int)));
    connect(bodyUI->reconnectButton, &QPushButton::clicked, this, &GeneralForm::onReconnectClicked);
    connect(bodyUI->cbFauxOfflineMessaging, &QCheckBox::stateChanged, this, &GeneralForm::onFauxOfflineMessaging);
    connect(bodyUI->cbCompactLayout, &QCheckBox::stateChanged, this, &GeneralForm::onCompactLayout);

#ifndef QTOX_PLATFORM_EXT
    bodyUI->autoAwayLabel->setEnabled(false);   // these don't seem to change the appearance of the widgets,
    bodyUI->autoAwaySpinBox->setEnabled(false); // though they are unusable
#endif
}

GeneralForm::~GeneralForm()
{
    delete bodyUI;
}

void GeneralForm::onEnableIPv6Updated()
{
    Settings::getInstance().setEnableIPv6(bodyUI->cbEnableIPv6->isChecked());
}

void GeneralForm::onTranslationUpdated()
{
    Settings::getInstance().setTranslation(locales[bodyUI->transComboBox->currentIndex()]);
    Widget::getInstance()->setTranslation();
}

void GeneralForm::onAutorunUpdated()
{
    Settings::getInstance().setAutorun(bodyUI->cbAutorun->isChecked());
}

void GeneralForm::onSetShowSystemTray()
{
    Settings::getInstance().setShowSystemTray(bodyUI->showSystemTray->isChecked());
    emit parent->setShowSystemTray(bodyUI->showSystemTray->isChecked());
}

void GeneralForm::onSetAutostartInTray()
{
    Settings::getInstance().setAutostartInTray(bodyUI->startInTray->isChecked());
}

void GeneralForm::onSetCloseToTray()
{
    Settings::getInstance().setCloseToTray(bodyUI->closeToTray->isChecked());
}

void GeneralForm::onSetLightTrayIcon()
{
    Settings::getInstance().setLightTrayIcon(bodyUI->lightTrayIcon->isChecked());
    Widget::getInstance()->updateIcons();
}

void GeneralForm::onSetMinimizeToTray()
{
    Settings::getInstance().setMinimizeToTray(bodyUI->minimizeToTray->isChecked());
}

void GeneralForm::onStyleSelected(QString style)
{
    if (bodyUI->styleBrowser->currentIndex() == 0)
        Settings::getInstance().setStyle("None");
    else
        Settings::getInstance().setStyle(style);

    this->setStyle(QStyleFactory::create(style));
    parent->setBodyHeadStyle(style);
}

void GeneralForm::onEmoticonSizeChanged()
{
    Settings::getInstance().setEmojiFontPointSize(bodyUI->emoticonSize->value());
}

void GeneralForm::onTimestampSelected(int index)
{
    Settings::getInstance().setTimestampFormat(timeFormats.at(index));
}

void GeneralForm::onAutoAwayChanged()
{
    int minutes = bodyUI->autoAwaySpinBox->value();
    Settings::getInstance().setAutoAwayTime(minutes);
}

void GeneralForm::onAutoAcceptFileChange()
{
    Settings::getInstance().setAutoSaveEnabled(bodyUI->autoacceptFiles->isChecked());

    if (bodyUI->autoacceptFiles->isChecked() == true)
        connect(bodyUI->autoSaveFilesDir, SIGNAL(clicked()), this, SLOT(onAutoSaveDirChange()));
    else
        disconnect(bodyUI->autoSaveFilesDir, SIGNAL(clicked()),this, SLOT(onAutoSaveDirChange()));
}

void GeneralForm::onAutoSaveDirChange()
{
    QString previousDir = Settings::getInstance().getGlobalAutoAcceptDir();
    QString directory = QFileDialog::getExistingDirectory(0, tr("Choose an auto accept directory","popup title"));
    if (directory.isEmpty())
        directory = previousDir;

    Settings::getInstance().setGlobalAutoAcceptDir(directory);
    bodyUI->autoSaveFilesDir->setText(directory);
}

void GeneralForm::onUseEmoticonsChange()
{
    Settings::getInstance().setUseEmoticons(bodyUI->useEmoticons->isChecked());
}

void GeneralForm::onSetStatusChange()
{
    Settings::getInstance().setStatusChangeNotificationEnabled(bodyUI->statusChanges->isChecked());
}

void GeneralForm::onSmileyBrowserIndexChanged(int index)
{
    QString filename = bodyUI->smileyPackBrowser->itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
    reloadSmiles();
}

void GeneralForm::onUDPUpdated()
{
    Settings::getInstance().setForceTCP(!bodyUI->cbEnableUDP->isChecked());
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
    int proxytype = bodyUI->proxyType->currentIndex();

    bodyUI->proxyAddr->setEnabled(proxytype);
    bodyUI->proxyPort->setEnabled(proxytype);
    Settings::getInstance().setProxyType(proxytype);
}

void GeneralForm::onReconnectClicked()
{
    if (Core::getInstance()->anyActiveCalls())
        QMessageBox::warning(this, tr("Call active", "popup title"),
           tr("You can't disconnect while a call is active!", "popup text"));
    else
        emit Widget::getInstance()->changeProfile(Settings::getInstance().getCurrentProfile());
}

void GeneralForm::reloadSmiles()
{
    QList<QStringList> emoticons = SmileyPack::getInstance().getEmoticons();
    QStringList smiles;
    smiles << ":)" << ";)" << ":p" << ":O" << ":["; //just in case...

    for (int i = 0; i < emoticons.size(); i++)
        smiles.push_front(emoticons.at(i).first());

    const QSize size(18,18);
    bodyUI->smile1->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[0]).pixmap(size));
    bodyUI->smile2->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[1]).pixmap(size));
    bodyUI->smile3->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[2]).pixmap(size));
    bodyUI->smile4->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[3]).pixmap(size));
    bodyUI->smile5->setPixmap(SmileyPack::getInstance().getAsIcon(smiles[4]).pixmap(size));
    
    bodyUI->smile1->setToolTip(smiles[0]);    
    bodyUI->smile2->setToolTip(smiles[1]);
    bodyUI->smile3->setToolTip(smiles[2]);
    bodyUI->smile4->setToolTip(smiles[3]);
    bodyUI->smile5->setToolTip(smiles[4]);
}

void GeneralForm::onCheckUpdateChanged()
{
    Settings::getInstance().setCheckUpdates(bodyUI->checkUpdates->isChecked());
}

void GeneralForm::onShowWindowChanged()
{
    Settings::getInstance().setShowWindow(bodyUI->showWindow->isChecked());
}

void GeneralForm::onSetShowInFront()
{
    Settings::getInstance().setShowInFront(bodyUI->showInFront->isChecked());
}

void GeneralForm::onSetGroupAlwaysNotify()
{
    Settings::getInstance().setGroupAlwaysNotify(bodyUI->groupAlwaysNotify->isChecked());
}

void GeneralForm::onFauxOfflineMessaging()
{
    Settings::getInstance().setFauxOfflineMessaging(bodyUI->cbFauxOfflineMessaging->isChecked());
}

void GeneralForm::onCompactLayout()
{
    Settings::getInstance().setCompactLayout(bodyUI->cbCompactLayout->isChecked());
    emit parent->compactToggled(bodyUI->cbCompactLayout->isChecked());
}

void GeneralForm::onThemeColorChanged(int)
{
    int index = bodyUI->themeColorCBox->currentIndex();
    Settings::getInstance().setThemeColor(index);
    Style::setThemeColor(index);
    Style::applyTheme();
}
