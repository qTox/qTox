/*
    Copyright © 2014-2019 by The qTox Project Contributors

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
#include <cmath>

#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

namespace {
// clang-format off
QStringList locales = {
    "ar",
    "be",
    "ber",
    "bg",
    "cs",
    "da",
    "de",
    "el",
    "en",
    "eo",
    "es",
    "et",
    "fa",
    "fi",
    "fr",
    "gl",
    "he",
    "hr",
    "hu",
    "is",
    "it",
    "ja",
    "jbo",
    "kn",
    "ko",
    "lt",
    "mk",
    "nl",
    "no_nb",
    "pl",
    "pr",
    "pt",
    "pt_BR",
    "ro",
    "ru",
    "si",
    "sk",
    "sl",
    "sq",
    "sr",
    "sr_Latn",
    "sv",
    "sw",
    "ta",
    "tr",
    "ug",
    "uk",
    "ur",
    "zh_CN",
    "zh_TW"
};
// clang-format on
} // namespace

/**
 * @class GeneralForm
 *
 * This form contains all settings that are not suited to other forms
 */
GeneralForm::GeneralForm(SettingsWidget* myParent, Settings& settings_, Style& style)
    : GenericForm(QPixmap(":/img/settings/general.png"), style)
    , bodyUI(new Ui::GeneralSettings)
    , settings{settings_}
{
    parent = myParent;

    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

#ifndef UPDATE_CHECK_ENABLED
    bodyUI->checkUpdates->setVisible(false);
#endif

#ifndef SPELL_CHECKING
    bodyUI->cbSpellChecking->setVisible(false);
#endif

    bodyUI->checkUpdates->setChecked(settings.getCheckUpdates());

    for (int i = 0; i < locales.size(); ++i) {
        QString langName;

        if (locales[i].startsWith(QLatin1String("eo"))) // QTBUG-57802
            langName = QLocale::languageToString(QLocale::Esperanto);
        else if (locales[i].startsWith(QLatin1String("jbo")))
            langName = QLatin1String("Lojban");
        else if (locales[i].startsWith(QLatin1String("pr")))
            langName = QLatin1String("Pirate");
        else if (locales[i] == (QLatin1String("pt"))) // QTBUG-47891
            langName = QStringLiteral("português");
        else
            langName = QLocale(locales[i]).nativeLanguageName();

        bodyUI->transComboBox->insertItem(i, langName);
    }

    bodyUI->transComboBox->setCurrentIndex(locales.indexOf(settings.getTranslation()));

    bodyUI->cbAutorun->setChecked(settings.getAutorun());

    bodyUI->cbSpellChecking->setChecked(settings.getSpellCheckingEnabled());
    bodyUI->lightTrayIcon->setChecked(settings.getLightTrayIcon());
    bool showSystemTray = settings.getShowSystemTray();

    bodyUI->showSystemTray->setChecked(showSystemTray);
    bodyUI->startInTray->setChecked(settings.getAutostartInTray());
    bodyUI->startInTray->setEnabled(showSystemTray);
    bodyUI->minimizeToTray->setChecked(settings.getMinimizeToTray());
    bodyUI->minimizeToTray->setEnabled(showSystemTray);
    bodyUI->closeToTray->setChecked(settings.getCloseToTray());
    bodyUI->closeToTray->setEnabled(showSystemTray);

    bodyUI->statusChanges->setChecked(settings.getStatusChangeNotificationEnabled());
    bodyUI->groupJoinLeaveMessages->setChecked(settings.getShowGroupJoinLeaveMessages());

    bodyUI->autoAwaySpinBox->setValue(settings.getAutoAwayTime());
    bodyUI->autoSaveFilesDir->setText(settings.getGlobalAutoAcceptDir());
    bodyUI->maxAutoAcceptSizeMB->setValue(static_cast<double>(settings.getMaxAutoAcceptSize()) / 1024 / 1024);
    bodyUI->autoacceptFiles->setChecked(settings.getAutoSaveEnabled());


#ifndef QTOX_PLATFORM_EXT
    bodyUI->autoAwayLabel->setEnabled(false); // these don't seem to change the appearance of the widgets,
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

void GeneralForm::on_transComboBox_currentIndexChanged(int index)
{
    const QString& locale = locales[index];
    settings.setTranslation(locale);
    Translator::translate(locale);
}

void GeneralForm::on_cbAutorun_stateChanged()
{
    settings.setAutorun(bodyUI->cbAutorun->isChecked());
}

void GeneralForm::on_cbSpellChecking_stateChanged()
{
    settings.setSpellCheckingEnabled(bodyUI->cbSpellChecking->isChecked());
}

void GeneralForm::on_showSystemTray_stateChanged()
{
    settings.setShowSystemTray(bodyUI->showSystemTray->isChecked());
    settings.saveGlobal();
}

void GeneralForm::on_startInTray_stateChanged()
{
    settings.setAutostartInTray(bodyUI->startInTray->isChecked());
}

void GeneralForm::on_closeToTray_stateChanged()
{
    settings.setCloseToTray(bodyUI->closeToTray->isChecked());
}

void GeneralForm::on_lightTrayIcon_stateChanged()
{
    settings.setLightTrayIcon(bodyUI->lightTrayIcon->isChecked());
    emit updateIcons();
}

void GeneralForm::on_minimizeToTray_stateChanged()
{
    settings.setMinimizeToTray(bodyUI->minimizeToTray->isChecked());
}

void GeneralForm::on_statusChanges_stateChanged()
{
    settings.setStatusChangeNotificationEnabled(bodyUI->statusChanges->isChecked());
}

void GeneralForm::on_groupJoinLeaveMessages_stateChanged()
{
    settings.setShowGroupJoinLeaveMessages(bodyUI->groupJoinLeaveMessages->isChecked());
}

void GeneralForm::on_autoAwaySpinBox_editingFinished()
{
    int minutes = bodyUI->autoAwaySpinBox->value();
    settings.setAutoAwayTime(minutes);
}

void GeneralForm::on_autoacceptFiles_stateChanged()
{
    settings.setAutoSaveEnabled(bodyUI->autoacceptFiles->isChecked());
}

void GeneralForm::on_autoSaveFilesDir_clicked()
{
    QString previousDir = settings.getGlobalAutoAcceptDir();
    QString directory =
        QFileDialog::getExistingDirectory(Q_NULLPTR,
                                          tr("Choose an auto accept directory", "popup title"),
                                          QDir::homePath());
    if (directory.isEmpty()) // cancel was pressed
        directory = previousDir;

    settings.setGlobalAutoAcceptDir(directory);
    bodyUI->autoSaveFilesDir->setText(directory);
}

void GeneralForm::on_maxAutoAcceptSizeMB_editingFinished()
{
    auto newMaxSizeMB = bodyUI->maxAutoAcceptSizeMB->value();
    auto newMaxSizeB = std::lround(newMaxSizeMB * 1024 * 1024);

    settings.setMaxAutoAcceptSize(newMaxSizeB);
}

void GeneralForm::on_checkUpdates_stateChanged()
{
    settings.setCheckUpdates(bodyUI->checkUpdates->isChecked());
}

/**
 * @brief Retranslate all elements in the form.
 */
void GeneralForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}
