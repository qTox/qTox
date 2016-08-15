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

#include "userinterfaceform.h"
#include "ui_userinterfacesettings.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTime>
#include <QVector>

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

/**
 * @class UserInterfaceForm
 *
 * This form contains all settings which change the UI appearance or behaviour.
 * It also contains the smiley configuration.
 */

static QStringList timeFormats = {"hh:mm AP", "hh:mm", "hh:mm:ss AP", "hh:mm:ss"};

// http://doc.qt.io/qt-4.8/qdate.html#fromString
static QStringList dateFormats = {"yyyy-MM-dd", "dd-MM-yyyy", "d-MM-yyyy", "dddd d-MM-yyyy", "dddd d-MM", "dddd dd MMMM"};

/**
 * @brief UserInterfaceForm::UserInterfaceForm
 * @param myParent Setting widget which will contain this form as tab.
 *
 * Constructor of UserInterfaceForm. Restores all controls from the settings.
 */
UserInterfaceForm::UserInterfaceForm(SettingsWidget* myParent) :
    GenericForm(QPixmap(":/img/settings/general.png"))
{
    parent = myParent;

    bodyUI = new Ui::UserInterfaceSettings;
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    Settings &s = Settings::getInstance();
    const QFont chatBaseFont = s.getChatMessageFont();
    bodyUI->txtChatFontSize->setValue(QFontInfo(chatBaseFont).pixelSize());
    bodyUI->txtChatFont->setCurrentFont(chatBaseFont);
    int index = static_cast<int>(s.getStylePreference());
    bodyUI->textStyleComboBox->setCurrentIndex(index);

    bool showWindow = s.getShowWindow();

    bodyUI->showWindow->setChecked(showWindow);
    bodyUI->showInFront->setChecked(s.getShowInFront());
    bodyUI->showInFront->setEnabled(showWindow);

    bodyUI->groupAlwaysNotify->setChecked(s.getGroupAlwaysNotify());
    bodyUI->cbGroupchatPosition->setChecked(s.getGroupchatPosition());
    bodyUI->cbCompactLayout->setChecked(s.getCompactLayout());
    bodyUI->cbSeparateWindow->setChecked(s.getSeparateWindow());
    bodyUI->cbDontGroupWindows->setChecked(s.getDontGroupWindows());
    bodyUI->cbDontGroupWindows->setEnabled(s.getSeparateWindow());

    bodyUI->useEmoticons->setChecked(s.getUseEmoticons());
    for (auto entry : SmileyPack::listSmileyPacks())
        bodyUI->smileyPackBrowser->addItem(entry.first, entry.second);

    smileLabels = {bodyUI->smile1, bodyUI->smile2, bodyUI->smile3,
                   bodyUI->smile4, bodyUI->smile5};

    int currentPack = bodyUI->smileyPackBrowser->findData(s.getSmileyPack());
    bodyUI->smileyPackBrowser->setCurrentIndex(currentPack);
    reloadSmiles();
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());

    bodyUI->styleBrowser->addItem(tr("None"));
    bodyUI->styleBrowser->addItems(QStyleFactory::keys());

    QString style;
    if (QStyleFactory::keys().contains(s.getStyle()))
        style = s.getStyle();
    else
        style = tr("None");

    bodyUI->styleBrowser->setCurrentText(style);

    for (QString color : Style::getThemeColorNames())
        bodyUI->themeColorCBox->addItem(color);

    bodyUI->themeColorCBox->setCurrentIndex(s.getThemeColor());
    bodyUI->emoticonSize->setValue(s.getEmojiFontPointSize());

    QStringList timestamps;
    for (QString timestamp : timeFormats)
        timestamps << QString("%1 - %2").arg(timestamp, QTime::currentTime().toString(timestamp));

    bodyUI->timestamp->addItems(timestamps);

    QLocale ql;
    QStringList datestamps;
    dateFormats.append(ql.dateFormat());
    dateFormats.append(ql.dateFormat(QLocale::LongFormat));
    dateFormats.removeDuplicates();
    timeFormats.append(ql.timeFormat());
    timeFormats.append(ql.timeFormat(QLocale::LongFormat));
    timeFormats.removeDuplicates();

    for (QString datestamp : dateFormats)
        datestamps << QString("%1 - %2").arg(datestamp, QDate::currentDate().toString(datestamp));

    bodyUI->dateFormats->addItems(datestamps);
    bodyUI->timestamp->setCurrentText(QString("%1 - %2").arg(s.getTimestampFormat(), QTime::currentTime().toString(s.getTimestampFormat())));
    bodyUI->dateFormats->setCurrentText(QString("%1 - %2").arg(s.getDateFormat(), QDate::currentDate().toString(s.getDateFormat())));

    eventsInit();
    Translator::registerHandler(std::bind(&UserInterfaceForm::retranslateUi, this), this);
}

UserInterfaceForm::~UserInterfaceForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void UserInterfaceForm::on_styleBrowser_currentIndexChanged(QString style)
{
    if (bodyUI->styleBrowser->currentIndex() == 0)
        Settings::getInstance().setStyle("None");
    else
        Settings::getInstance().setStyle(style);

    this->setStyle(QStyleFactory::create(style));
    parent->setBodyHeadStyle(style);
}

void UserInterfaceForm::on_emoticonSize_editingFinished()
{
    Settings::getInstance().setEmojiFontPointSize(bodyUI->emoticonSize->value());
}

void UserInterfaceForm::on_timestamp_currentIndexChanged(int index)
{
    Settings::getInstance().setTimestampFormat(timeFormats.at(index));
    Translator::translate();
}

void UserInterfaceForm::on_dateFormats_currentIndexChanged(int index)
{
    Settings::getInstance().setDateFormat(dateFormats.at(index));
    Translator::translate();
}

void UserInterfaceForm::on_useEmoticons_stateChanged()
{
    Settings::getInstance().setUseEmoticons(bodyUI->useEmoticons->isChecked());
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());
}

void UserInterfaceForm::on_textStyleComboBox_currentTextChanged()
{
    Settings::StyleType styleType =
            static_cast<Settings::StyleType>(bodyUI->textStyleComboBox->currentIndex());
    Settings::getInstance().setStylePreference(styleType);
}

void UserInterfaceForm::on_smileyPackBrowser_currentIndexChanged(int index)
{
    QString filename = bodyUI->smileyPackBrowser->itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
    reloadSmiles();
}

/**
 * @brief Reload smiles and size information.
 */
void UserInterfaceForm::reloadSmiles()
{
    QList<QStringList> emoticons = SmileyPack::getInstance().getEmoticons();

    // sometimes there are no emoticons available, don't crash in this case
    if (emoticons.isEmpty())
    {
        qDebug() << "reloadSmilies: No emoticons found";
        return;
    }

    QStringList smiles;
    for (int i = 0; i < emoticons.size(); i++)
        smiles.push_front(emoticons.at(i).first());

    const QSize size(18,18);
    for (int i = 0; i < smileLabels.size(); i++)
    {
        QIcon icon = SmileyPack::getInstance().getAsIcon(smiles[i]);
        smileLabels[i]->setPixmap(icon.pixmap(size));
        smileLabels[i]->setToolTip(smiles[i]);
    }

    //set maximum size of emoji
    QDesktopWidget desktop;
    // 8 is the count of row and column in emoji's in widget
    const int sideSize = 8;
    int maxSide = qMin(desktop.geometry().height() / sideSize,
                       desktop.geometry().width() / sideSize);
    QSize maxSize(maxSide, maxSide);

    QIcon icon = SmileyPack::getInstance().getAsIcon(smiles[0]);
    QSize actualSize = icon.actualSize(maxSize);
    bodyUI->emoticonSize->setMaximum(actualSize.width());
}

void UserInterfaceForm::on_showWindow_stateChanged()
{
    bool isChecked = bodyUI->showWindow->isChecked();
    Settings::getInstance().setShowWindow(isChecked);
    bodyUI->showInFront->setEnabled(isChecked);
}

void UserInterfaceForm::on_showInFront_stateChanged()
{
    Settings::getInstance().setShowInFront(bodyUI->showInFront->isChecked());
}

void UserInterfaceForm::on_groupAlwaysNotify_stateChanged()
{
    Settings::getInstance().setGroupAlwaysNotify(bodyUI->groupAlwaysNotify->isChecked());
}

void UserInterfaceForm::on_cbCompactLayout_stateChanged()
{
    Settings::getInstance().setCompactLayout(bodyUI->cbCompactLayout->isChecked());
}

void UserInterfaceForm::on_cbSeparateWindow_stateChanged()
{
    bool checked = bodyUI->cbSeparateWindow->isChecked();
    bodyUI->cbDontGroupWindows->setEnabled(checked);
    Settings::getInstance().setSeparateWindow(checked);
}

void UserInterfaceForm::on_cbDontGroupWindows_stateChanged()
{
    Settings::getInstance().setDontGroupWindows(bodyUI->cbDontGroupWindows->isChecked());
}

void UserInterfaceForm::on_cbGroupchatPosition_stateChanged()
{
    Settings::getInstance().setGroupchatPosition(bodyUI->cbGroupchatPosition->isChecked());
}

void UserInterfaceForm::on_themeColorCBox_currentIndexChanged(int)
{
    int index = bodyUI->themeColorCBox->currentIndex();
    Settings::getInstance().setThemeColor(index);
    Style::setThemeColor(index);
    Style::applyTheme();
}

/**
 * @brief Retranslate all elements on the form.
 */
void UserInterfaceForm::retranslateUi()
{
    bodyUI->retranslateUi(this);

    QStringList colorThemes(Style::getThemeColorNames());
    for (int i = 0; i < colorThemes.size(); ++i)
    {
        bodyUI->themeColorCBox->setItemText(i, colorThemes[i]);
    }

    bodyUI->styleBrowser->setItemText(0, tr("None"));
}

void UserInterfaceForm::on_txtChatFont_currentFontChanged(const QFont& f)
{
    QFont tmpFont = f;
    const int px = bodyUI->txtChatFontSize->value();

    if (QFontInfo(tmpFont).pixelSize() != px)
        tmpFont.setPixelSize(px);

    Settings::getInstance().setChatMessageFont(tmpFont);
}

void UserInterfaceForm::on_txtChatFontSize_valueChanged(int px)
{
    Settings& s = Settings::getInstance();
    QFont tmpFont = s.getChatMessageFont();
    const int fontSize = QFontInfo(tmpFont).pixelSize();

    if (px != fontSize)
    {
        tmpFont.setPixelSize(px);
        s.setChatMessageFont(tmpFont);
    }
}
