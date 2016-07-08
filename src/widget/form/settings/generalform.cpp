/*
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "generalform.h"
#include "ui_generalsettings.h"

#include <QFileDialog>

#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/recursivesignalblocker.h"
#include "src/net/autoupdate.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

static QStringList locales = {"ar",
                              "be",
                              "bg",
                              "cs",
                              "da",
                              "de",
                              "et",
                              "el",
                              "en",
                              "es",
                              "eo",
                              "fr",
                              "he",
                              "hr",
                              "it",
                              "lt",
                              "jbo",
                              "hu",
                              "nl",
                              "ja",
                              "no_nb",
                              "pl",
                              "pt",
                              "ru",
                              "sl",
                              "fi",
                              "sv",
                              "tr",
                              "ug",
                              "uk",
                              "zh"};
static QStringList langs = {"Arabic",
                            "Беларуская",
                            "Български",
                            "Čeština",
                            "Dansk",
                            "Deutsch",
                            "Eesti",
                            "Ελληνικά",
                            "English",
                            "Español",
                            "Esperanto",
                            "Français",
                            "עברית",
                            "Hrvatski",
                            "Italiano",
                            "Lietuvių",
                            "Lojban",
                            "Magyar",
                            "Nederlands",
                            "日本語",
                            "Norsk Bokmål",
                            "Polski",
                            "Português",
                            "Русский",
                            "Slovenščina",
                            "Suomi",
                            "Svenska",
                            "Türkçe",
                            "ئۇيغۇرچە",
                            "Українська",
                            "简体中文"};

GeneralForm::GeneralForm(SettingsWidget *myParent)
    : GenericForm(QPixmap(":/img/settings/general.png"))
    , bodyUI(new Ui::GeneralSettings)
{
    parent = myParent;

    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    Settings& s = Settings::getInstance();

    bodyUI->checkUpdates->setVisible(AUTOUPDATE_ENABLED);
    bodyUI->checkUpdates->setChecked(s.getCheckUpdates());

    for (int i = 0; i < langs.size(); i++)
        bodyUI->transComboBox->insertItem(i, langs[i]);

    bodyUI->transComboBox->setCurrentIndex(locales.indexOf(s.getTranslation()));

    bodyUI->cbAutorun->setChecked(s.getAutorun());

    bodyUI->lightTrayIcon->setChecked(s.getLightTrayIcon());
    bool showSystemTray = s.getShowSystemTray();

    bodyUI->showSystemTray->setChecked(showSystemTray);
    bodyUI->startInTray->setChecked(s.getAutostartInTray());
    bodyUI->startInTray->setEnabled(showSystemTray);
    bodyUI->minimizeToTray->setChecked(s.getMinimizeToTray());
    bodyUI->minimizeToTray->setEnabled(showSystemTray);
    bodyUI->closeToTray->setChecked(s.getCloseToTray());
    bodyUI->closeToTray->setEnabled(showSystemTray);

    bodyUI->notifySound->setChecked(s.getNotifySound());
    bodyUI->busySound->setChecked(s.getBusySound());
    bodyUI->busySound->setEnabled(s.getNotifySound());

    bodyUI->statusChanges->setChecked(s.getStatusChangeNotificationEnabled());
    bodyUI->cbFauxOfflineMessaging->setChecked(s.getFauxOfflineMessaging());

    bodyUI->autoAwaySpinBox->setValue(s.getAutoAwayTime());
    bodyUI->autoSaveFilesDir->setText(s.getGlobalAutoAcceptDir());
    bodyUI->autoacceptFiles->setChecked(s.getAutoSaveEnabled());

    // General
    void (QComboBox::* currentIndexChanged)(int index) = &QComboBox::currentIndexChanged;
    connect(bodyUI->transComboBox, currentIndexChanged, this, &GeneralForm::onTranslationUpdated);
    connect(bodyUI->checkUpdates, &QCheckBox::stateChanged, this, &GeneralForm::onCheckUpdateChanged);
    connect(bodyUI->cbAutorun, &QCheckBox::stateChanged, this, &GeneralForm::onAutorunUpdated);
    connect(bodyUI->lightTrayIcon, &QCheckBox::stateChanged, this, &GeneralForm::onSetLightTrayIcon);

    connect(bodyUI->showSystemTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetShowSystemTray);
    connect(bodyUI->startInTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetAutostartInTray);
    connect(bodyUI->minimizeToTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetMinimizeToTray);
    connect(bodyUI->closeToTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetCloseToTray);

    connect(bodyUI->notifySound, &QCheckBox::stateChanged, this, &GeneralForm::onSetNotifySound);
    connect(bodyUI->busySound, &QCheckBox::stateChanged, this, &GeneralForm::onSetBusySound);
    connect(bodyUI->statusChanges, &QCheckBox::stateChanged, this, &GeneralForm::onSetStatusChange);
    connect(bodyUI->cbFauxOfflineMessaging, &QCheckBox::stateChanged, this, &GeneralForm::onFauxOfflineMessaging);

    connect(bodyUI->autoAwaySpinBox, &QSpinBox::editingFinished, this, &GeneralForm::onAutoAwayChanged);
    connect(bodyUI->autoSaveFilesDir, &QPushButton::clicked, this, &GeneralForm::onAutoSaveDirChange);
    connect(bodyUI->autoacceptFiles, &QCheckBox::stateChanged, this, &GeneralForm::onAutoAcceptFileChange);

#ifndef QTOX_PLATFORM_EXT
    bodyUI->autoAwayLabel->setEnabled(false);   // these don't seem to change the appearance of the widgets,
    bodyUI->autoAwaySpinBox->setEnabled(false); // though they are unusable
#endif

    eventsInit();
    Translator::registerHandler(std::bind(&GeneralForm::retranslateUi, this), this);
}

GeneralForm::~GeneralForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void GeneralForm::onTranslationUpdated()
{
    Settings::getInstance().setTranslation(locales[bodyUI->transComboBox->currentIndex()]);
    Translator::translate();
}

void GeneralForm::onAutorunUpdated()
{
    Settings::getInstance().setAutorun(bodyUI->cbAutorun->isChecked());
}

void GeneralForm::onSetShowSystemTray()
{
    Settings::getInstance().setShowSystemTray(bodyUI->showSystemTray->isChecked());
    Settings::getInstance().saveGlobal();
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

void GeneralForm::onSetNotifySound()
{
    bool notify = bodyUI->notifySound->isChecked();
    Settings::getInstance().setNotifySound(notify);
    bodyUI->busySound->setEnabled(notify);
}

void GeneralForm::onSetBusySound()
{
    Settings::getInstance().setBusySound(bodyUI->busySound->isChecked());
}

void GeneralForm::onSetStatusChange()
{
    Settings::getInstance().setStatusChangeNotificationEnabled(bodyUI->statusChanges->isChecked());
}

void GeneralForm::onFauxOfflineMessaging()
{
    Settings::getInstance().setFauxOfflineMessaging(bodyUI->cbFauxOfflineMessaging->isChecked());
}

void GeneralForm::onAutoAwayChanged()
{
    int minutes = bodyUI->autoAwaySpinBox->value();
    Settings::getInstance().setAutoAwayTime(minutes);
}

void GeneralForm::onAutoAcceptFileChange()
{
    Settings::getInstance().setAutoSaveEnabled(bodyUI->autoacceptFiles->isChecked());
}

void GeneralForm::onAutoSaveDirChange()
{
    QString previousDir = Settings::getInstance().getGlobalAutoAcceptDir();
    QString directory = QFileDialog::getExistingDirectory(0,
                                                          tr("Choose an auto accept directory", "popup title"),  //opens in home directory
                                                          QDir::homePath(),
                                                          QFileDialog::DontUseNativeDialog);
    if (directory.isEmpty())  // cancel was pressed
        directory = previousDir;

    Settings::getInstance().setGlobalAutoAcceptDir(directory);
    bodyUI->autoSaveFilesDir->setText(directory);
}

void GeneralForm::onCheckUpdateChanged()
{
    Settings::getInstance().setCheckUpdates(bodyUI->checkUpdates->isChecked());
}

void GeneralForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}
