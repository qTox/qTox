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

#include <QDesktopWidget>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTime>
#include <QFileDialog>
#include <QFont>
#include <QStandardPaths>
#include <QDebug>

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

static QStringList timeFormats = {"hh:mm AP", "hh:mm", "hh:mm:ss AP", "hh:mm:ss"};
// http://doc.qt.io/qt-4.8/qdate.html#fromString
static QStringList dateFormats = {"yyyy-MM-dd", "dd-MM-yyyy", "d-MM-yyyy", "dddd d-MM-yyyy", "dddd d-MM", "dddd dd MMMM"};

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

    bodyUI->smileyPackBrowser->setCurrentIndex(bodyUI->smileyPackBrowser->findData(s.getSmileyPack()));
    reloadSmiles();
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());

    bodyUI->styleBrowser->addItem(tr("None"));
    bodyUI->styleBrowser->addItems(QStyleFactory::keys());
    if (QStyleFactory::keys().contains(s.getStyle()))
        bodyUI->styleBrowser->setCurrentText(s.getStyle());
    else
        bodyUI->styleBrowser->setCurrentText(tr("None"));

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

    // Chat
    connect(bodyUI->textStyleComboBox, &QComboBox::currentTextChanged, this, &UserInterfaceForm::onStyleUpdated);

    connect(bodyUI->showWindow, &QCheckBox::stateChanged, this, &UserInterfaceForm::onShowWindowChanged);
    connect(bodyUI->showInFront, &QCheckBox::stateChanged, this, &UserInterfaceForm::onSetShowInFront);

    connect(bodyUI->groupAlwaysNotify, &QCheckBox::stateChanged, this, &UserInterfaceForm::onSetGroupAlwaysNotify);
    connect(bodyUI->cbGroupchatPosition, &QCheckBox::stateChanged, this, &UserInterfaceForm::onGroupchatPositionChanged);
    connect(bodyUI->cbCompactLayout, &QCheckBox::stateChanged, this, &UserInterfaceForm::onCompactLayout);

    connect(bodyUI->cbDontGroupWindows, &QCheckBox::stateChanged, this, &UserInterfaceForm::onDontGroupWindowsChanged);
    connect(bodyUI->cbSeparateWindow, &QCheckBox::stateChanged, this, &UserInterfaceForm::onSeparateWindowChanged);

    // Theme
    void (QComboBox::* currentIndexChanged)(int) = &QComboBox::currentIndexChanged;
    connect(bodyUI->useEmoticons, &QCheckBox::stateChanged, this, &UserInterfaceForm::onUseEmoticonsChange);
    connect(bodyUI->smileyPackBrowser, currentIndexChanged, this, &UserInterfaceForm::onSmileyBrowserIndexChanged);
    connect(bodyUI->styleBrowser, &QComboBox::currentTextChanged, this, &UserInterfaceForm::onStyleSelected);
    connect(bodyUI->themeColorCBox, currentIndexChanged, this, &UserInterfaceForm::onThemeColorChanged);
    connect(bodyUI->emoticonSize, &QSpinBox::editingFinished, this, &UserInterfaceForm::onEmoticonSizeChanged);
    connect(bodyUI->timestamp, currentIndexChanged, this, &UserInterfaceForm::onTimestampSelected);
    connect(bodyUI->dateFormats, currentIndexChanged, this, &UserInterfaceForm::onDateFormatSelected);


    eventsInit();
    Translator::registerHandler(std::bind(&UserInterfaceForm::retranslateUi, this), this);
}

UserInterfaceForm::~UserInterfaceForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void UserInterfaceForm::onStyleSelected(QString style)
{
    if (bodyUI->styleBrowser->currentIndex() == 0)
        Settings::getInstance().setStyle("None");
    else
        Settings::getInstance().setStyle(style);

    this->setStyle(QStyleFactory::create(style));
    parent->setBodyHeadStyle(style);
}

void UserInterfaceForm::onEmoticonSizeChanged()
{
    Settings::getInstance().setEmojiFontPointSize(bodyUI->emoticonSize->value());
}

void UserInterfaceForm::onTimestampSelected(int index)
{
    Settings::getInstance().setTimestampFormat(timeFormats.at(index));
    Translator::translate();
}

void UserInterfaceForm::onDateFormatSelected(int index)
{
    Settings::getInstance().setDateFormat(dateFormats.at(index));
    Translator::translate();
}

void UserInterfaceForm::onUseEmoticonsChange()
{
    Settings::getInstance().setUseEmoticons(bodyUI->useEmoticons->isChecked());
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());
}

void UserInterfaceForm::onStyleUpdated()
{
    Settings::StyleType styleType =
            static_cast<Settings::StyleType>(bodyUI->textStyleComboBox->currentIndex());
    Settings::getInstance().setStylePreference(styleType);
}

void UserInterfaceForm::onSmileyBrowserIndexChanged(int index)
{
    QString filename = bodyUI->smileyPackBrowser->itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
    reloadSmiles();
}

void UserInterfaceForm::reloadSmiles()
{
    QList<QStringList> emoticons = SmileyPack::getInstance().getEmoticons();
    if (emoticons.isEmpty())
    { // sometimes there are no emoticons available, don't crash in this case
        qDebug() << "reloadSmilies: No emoticons found";
        return;
    }

    QStringList smiles;
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

    //set maximum size of emoji
    QDesktopWidget desktop;
    int maxSize = qMin(desktop.geometry().height()/8,
                       desktop.geometry().width()/8); // 8 is the count of row and column in emoji's in widget
    bodyUI->emoticonSize->setMaximum(SmileyPack::getInstance().getAsIcon(smiles[0]).actualSize(QSize(maxSize,maxSize)).width());
}

void UserInterfaceForm::onShowWindowChanged()
{
    Settings::getInstance().setShowWindow(bodyUI->showWindow->isChecked());
}

void UserInterfaceForm::onSetShowInFront()
{
    Settings::getInstance().setShowInFront(bodyUI->showInFront->isChecked());
}

void UserInterfaceForm::onSetGroupAlwaysNotify()
{
    Settings::getInstance().setGroupAlwaysNotify(bodyUI->groupAlwaysNotify->isChecked());
}

void UserInterfaceForm::onCompactLayout()
{
    Settings::getInstance().setCompactLayout(bodyUI->cbCompactLayout->isChecked());
}

void UserInterfaceForm::onSeparateWindowChanged()
{
    bool checked = bodyUI->cbSeparateWindow->isChecked();
    bodyUI->cbDontGroupWindows->setEnabled(checked);
    Settings::getInstance().setSeparateWindow(checked);
}

void UserInterfaceForm::onDontGroupWindowsChanged()
{
    Settings::getInstance().setDontGroupWindows(bodyUI->cbDontGroupWindows->isChecked());
}

void UserInterfaceForm::onGroupchatPositionChanged()
{
    Settings::getInstance().setGroupchatPosition(bodyUI->cbGroupchatPosition->isChecked());
}

void UserInterfaceForm::onThemeColorChanged(int)
{
    int index = bodyUI->themeColorCBox->currentIndex();
    Settings::getInstance().setThemeColor(index);
    Style::setThemeColor(index);
    Style::applyTheme();
}

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
