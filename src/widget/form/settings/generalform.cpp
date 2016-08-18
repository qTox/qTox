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

#include <src/core/recursivesignalblocker.h>
#include "src/widget/form/settingswidget.h"
#include "src/widget/widget.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/widget/style.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/widget/translator.h"
#include "src/net/autoupdate.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTime>
#include <QFileDialog>
#include <QFont>
#include <QStandardPaths>
#include <QDebug>

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

static QStringList timeFormats = {"hh:mm AP", "hh:mm", "hh:mm:ss AP", "hh:mm:ss"};
// http://doc.qt.io/qt-4.8/qdate.html#fromString
static QStringList dateFormats = {"yyyy-MM-dd", "dd-MM-yyyy", "d-MM-yyyy", "dddd d-MM-yyyy", "dddd d-MM", "dddd dd MMMM"};

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

    bodyUI->cbEnableIPv6->setChecked(s.getEnableIPv6());
    for (int i = 0; i < langs.size(); i++)
        bodyUI->transComboBox->insertItem(i, langs[i]);

    bodyUI->transComboBox->setCurrentIndex(locales.indexOf(s.getTranslation()));

    const QFont chatBaseFont = s.getChatMessageFont();
    bodyUI->txtChatFontSize->setValue(QFontInfo(chatBaseFont).pixelSize());
    bodyUI->txtChatFont->setCurrentFont(chatBaseFont);
    bodyUI->textStyleComboBox->setCurrentIndex(static_cast<int>(s.getStylePreference()));
    bodyUI->cbAutorun->setChecked(s.getAutorun());

    bool showSystemTray = s.getShowSystemTray();

    bodyUI->showSystemTray->setChecked(showSystemTray);
    bodyUI->startInTray->setChecked(s.getAutostartInTray());
    bodyUI->startInTray->setEnabled(showSystemTray);
    bodyUI->closeToTray->setChecked(s.getCloseToTray());
    bodyUI->closeToTray->setEnabled(showSystemTray);
    bodyUI->minimizeToTray->setChecked(s.getMinimizeToTray());
    bodyUI->minimizeToTray->setEnabled(showSystemTray);
    bodyUI->lightTrayIcon->setChecked(s.getLightTrayIcon());

    bodyUI->statusChanges->setChecked(s.getStatusChangeNotificationEnabled());
    bodyUI->useEmoticons->setChecked(s.getUseEmoticons());
    bodyUI->autoacceptFiles->setChecked(s.getAutoSaveEnabled());
    bodyUI->autoSaveFilesDir->setText(s.getGlobalAutoAcceptDir());

    bool showWindow = s.getShowWindow();

    bodyUI->showWindow->setChecked(showWindow);
    bodyUI->showInFront->setChecked(s.getShowInFront());
    bodyUI->showInFront->setEnabled(showWindow);

    bool notifySound = s.getNotifySound();

    bodyUI->notifySound->setChecked(notifySound);
    bodyUI->busySound->setChecked(s.getBusySound());
    bodyUI->busySound->setEnabled(notifySound);
    bodyUI->groupAlwaysNotify->setChecked(s.getGroupAlwaysNotify());
    bodyUI->cbFauxOfflineMessaging->setChecked(s.getFauxOfflineMessaging());
    bodyUI->cbCompactLayout->setChecked(s.getCompactLayout());
    bodyUI->cbSeparateWindow->setChecked(s.getSeparateWindow());
    bodyUI->cbDontGroupWindows->setChecked(s.getDontGroupWindows());
    bodyUI->cbDontGroupWindows->setEnabled(bodyUI->cbSeparateWindow->isChecked());
    bodyUI->cbGroupchatPosition->setChecked(s.getGroupchatPosition());

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

    bodyUI->autoAwaySpinBox->setValue(s.getAutoAwayTime());

    bodyUI->cbEnableUDP->setChecked(!s.getForceTCP());
    bodyUI->proxyAddr->setText(s.getProxyAddr());
    int port = s.getProxyPort();
    if (port != -1)
        bodyUI->proxyPort->setValue(port);

    bodyUI->proxyType->setCurrentIndex(static_cast<int>(s.getProxyType()));
    onUseProxyUpdated();

    //general
    connect(bodyUI->checkUpdates, &QCheckBox::stateChanged, this, &GeneralForm::onCheckUpdateChanged);
    connect(bodyUI->transComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTranslationUpdated()));
    connect(bodyUI->cbAutorun, &QCheckBox::stateChanged, this, &GeneralForm::onAutorunUpdated);
    connect(bodyUI->lightTrayIcon, &QCheckBox::stateChanged, this, &GeneralForm::onSetLightTrayIcon);
    connect(bodyUI->showSystemTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetShowSystemTray);
    connect(bodyUI->startInTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetAutostartInTray);
    connect(bodyUI->closeToTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetCloseToTray);
    connect(bodyUI->minimizeToTray, &QCheckBox::stateChanged, this, &GeneralForm::onSetMinimizeToTray);
    connect(bodyUI->statusChanges, &QCheckBox::stateChanged, this, &GeneralForm::onSetStatusChange);
    connect(bodyUI->autoAwaySpinBox, SIGNAL(editingFinished()), this, SLOT(onAutoAwayChanged()));
    connect(bodyUI->showWindow, &QCheckBox::stateChanged, this, &GeneralForm::onShowWindowChanged);
    connect(bodyUI->showInFront, &QCheckBox::stateChanged, this, &GeneralForm::onSetShowInFront);
    connect(bodyUI->notifySound, &QCheckBox::stateChanged, this, &GeneralForm::onSetNotifySound);
    connect(bodyUI->busySound, &QCheckBox::stateChanged, this, &GeneralForm::onSetBusySound);
    connect(bodyUI->textStyleComboBox, &QComboBox::currentTextChanged, this, &GeneralForm::onStyleUpdated);
    connect(bodyUI->groupAlwaysNotify, &QCheckBox::stateChanged, this, &GeneralForm::onSetGroupAlwaysNotify);
    connect(bodyUI->autoacceptFiles, &QCheckBox::stateChanged, this, &GeneralForm::onAutoAcceptFileChange);
    connect(bodyUI->autoSaveFilesDir, SIGNAL(clicked()), this, SLOT(onAutoSaveDirChange()));
    //theme
    connect(bodyUI->useEmoticons, &QCheckBox::stateChanged, this, &GeneralForm::onUseEmoticonsChange);
    connect(bodyUI->smileyPackBrowser, SIGNAL(currentIndexChanged(int)), this, SLOT(onSmileyBrowserIndexChanged(int)));
    connect(bodyUI->styleBrowser, SIGNAL(currentTextChanged(QString)), this, SLOT(onStyleSelected(QString)));
    connect(bodyUI->themeColorCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onThemeColorChanged(int)));
    connect(bodyUI->emoticonSize, SIGNAL(editingFinished()), this, SLOT(onEmoticonSizeChanged()));
    connect(bodyUI->timestamp, SIGNAL(currentIndexChanged(int)), this, SLOT(onTimestampSelected(int)));
    connect(bodyUI->dateFormats, SIGNAL(currentIndexChanged(int)), this, SLOT(onDateFormatSelected(int)));
    //connection
    connect(bodyUI->cbEnableIPv6, &QCheckBox::stateChanged, this, &GeneralForm::onEnableIPv6Updated);
    connect(bodyUI->cbEnableUDP, &QCheckBox::stateChanged, this, &GeneralForm::onUDPUpdated);
    connect(bodyUI->proxyType, SIGNAL(currentIndexChanged(int)), this, SLOT(onUseProxyUpdated()));
    connect(bodyUI->proxyAddr, &QLineEdit::editingFinished, this, &GeneralForm::onProxyAddrEdited);
    connect(bodyUI->proxyPort, SIGNAL(valueChanged(int)), this, SLOT(onProxyPortEdited(int)));
    connect(bodyUI->reconnectButton, &QPushButton::clicked, this, &GeneralForm::onReconnectClicked);
    connect(bodyUI->cbFauxOfflineMessaging, &QCheckBox::stateChanged, this, &GeneralForm::onFauxOfflineMessaging);
    connect(bodyUI->cbCompactLayout, &QCheckBox::stateChanged, this, &GeneralForm::onCompactLayout);
    connect(bodyUI->cbSeparateWindow, &QCheckBox::stateChanged, this, &GeneralForm::onSeparateWindowChanged);
    connect(bodyUI->cbDontGroupWindows, &QCheckBox::stateChanged, this, &GeneralForm::onDontGroupWindowsChanged);
    connect(bodyUI->cbGroupchatPosition, &QCheckBox::stateChanged, this, &GeneralForm::onGroupchatPositionChanged);

    // prevent stealing mouse wheel scroll
    // scrolling event won't be transmitted to comboboxes or qspinboxes when scrolling
    // you can scroll through general settings without accidentially changing theme/skin/icons etc.
    // @see GeneralForm::eventFilter(QObject *o, QEvent *e) at the bottom of this file for more
    for (QComboBox *cb : findChildren<QComboBox*>())
    {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }

    for (QSpinBox *sp : findChildren<QSpinBox*>())
    {
        sp->installEventFilter(this);
        sp->setFocusPolicy(Qt::WheelFocus);
    }

    for (QCheckBox *cb : findChildren<QCheckBox*>()) // this one is to allow scrolling on checkboxes
    {
        cb->installEventFilter(this);
    }

#ifndef QTOX_PLATFORM_EXT
    bodyUI->autoAwayLabel->setEnabled(false);   // these don't seem to change the appearance of the widgets,
    bodyUI->autoAwaySpinBox->setEnabled(false); // though they are unusable
#endif

    Translator::registerHandler(std::bind(&GeneralForm::retranslateUi, this), this);
}

GeneralForm::~GeneralForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void GeneralForm::onEnableIPv6Updated()
{
    Settings::getInstance().setEnableIPv6(bodyUI->cbEnableIPv6->isChecked());
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
    Translator::translate();
}

void GeneralForm::onDateFormatSelected(int index)
{
    Settings::getInstance().setDateFormat(dateFormats.at(index));
    Translator::translate();
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

void GeneralForm::onUseEmoticonsChange()
{
    Settings::getInstance().setUseEmoticons(bodyUI->useEmoticons->isChecked());
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());
}

void GeneralForm::onStyleUpdated()
{
    Settings::StyleType styleType =
            static_cast<Settings::StyleType>(bodyUI->textStyleComboBox->currentIndex());
    Settings::getInstance().setStylePreference(styleType);
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
    Settings::getInstance().setProxyPort(static_cast<quint16>(port));
}

void GeneralForm::onUseProxyUpdated()
{
    Settings::ProxyType proxytype =
            static_cast<Settings::ProxyType>(bodyUI->proxyType->currentIndex());

    bodyUI->proxyAddr->setEnabled(proxytype != Settings::ProxyType::ptNone);
    bodyUI->proxyPort->setEnabled(proxytype != Settings::ProxyType::ptNone);
    Settings::getInstance().setProxyType(proxytype);
}

void GeneralForm::onReconnectClicked()
{
    if (Core::getInstance()->getAv()->anyActiveCalls())
    {
        QMessageBox::warning(this, tr("Call active", "popup title"),
                        tr("You can't disconnect while a call is active!", "popup text"));
    }
    else
    {
        emit Core::getInstance()->statusSet(Status::Offline);
        Nexus::getProfile()->restartCore();
    }
}

void GeneralForm::reloadSmiles()
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

void GeneralForm::onSetNotifySound()
{
    Settings::getInstance().setNotifySound(bodyUI->notifySound->isChecked());
}

void GeneralForm::onSetBusySound()
{
    Settings::getInstance().setBusySound(bodyUI->busySound->isChecked());
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
}

void GeneralForm::onSeparateWindowChanged()
{
    bodyUI->cbDontGroupWindows->setEnabled(bodyUI->cbSeparateWindow->isChecked());
    Settings::getInstance().setSeparateWindow(bodyUI->cbSeparateWindow->isChecked());
}

void GeneralForm::onDontGroupWindowsChanged()
{
    Settings::getInstance().setDontGroupWindows(bodyUI->cbDontGroupWindows->isChecked());
}

void GeneralForm::onGroupchatPositionChanged()
{
    Settings::getInstance().setGroupchatPosition(bodyUI->cbGroupchatPosition->isChecked());
}

void GeneralForm::onThemeColorChanged(int)
{
    int index = bodyUI->themeColorCBox->currentIndex();
    Settings::getInstance().setThemeColor(index);
    Style::setThemeColor(index);
    Style::applyTheme();
}

bool GeneralForm::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Wheel) &&
         (qobject_cast<QComboBox*>(o) || qobject_cast<QAbstractSpinBox*>(o) || qobject_cast<QCheckBox*>(o)))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}

void GeneralForm::retranslateUi()
{
    int proxyType = bodyUI->proxyType->currentIndex();
    bodyUI->retranslateUi(this);
    bodyUI->proxyType->setCurrentIndex(proxyType);

    QStringList colorThemes(Style::getThemeColorNames());
    for (int i = 0; i != colorThemes.size(); ++i)
    {
        bodyUI->themeColorCBox->setItemText(i, colorThemes[i]);
    }

    bodyUI->styleBrowser->setItemText(0, tr("None"));
}

void GeneralForm::on_txtChatFont_currentFontChanged(const QFont& f)
{
    QFont tmpFont = f;
    const int px = bodyUI->txtChatFontSize->value();

    if (QFontInfo(tmpFont).pixelSize() != px)
        tmpFont.setPixelSize(px);

    Settings::getInstance().setChatMessageFont(tmpFont);
}

void GeneralForm::on_txtChatFontSize_valueChanged(int px)
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
