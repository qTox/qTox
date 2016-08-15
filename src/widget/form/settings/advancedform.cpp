/*
    Copyright © 2014-2016 by The qTox Project

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

#include "advancedform.h"
#include "ui_advancedsettings.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QProcess>

#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/recursivesignalblocker.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/persistence/db/plaindb.h"
#include "src/persistence/profile.h"
#include "src/widget/gui.h"
#include "src/widget/translator.h"

AdvancedForm::AdvancedForm()
  : GenericForm(QPixmap(":/img/settings/general.png"))
  , bodyUI (new Ui::AdvancedSettings)
{
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    Settings &s = Settings::getInstance();
    bodyUI->cbEnableIPv6->setChecked(s.getEnableIPv6());
    bodyUI->cbMakeToxPortable->setChecked(Settings::getInstance().getMakeToxPortable());
    bodyUI->cbEnableUDP->setChecked(!s.getForceTCP());
    bodyUI->proxyAddr->setText(s.getProxyAddr());
    quint16 port = s.getProxyPort();
    if (port > 0)
        bodyUI->proxyPort->setValue(port);

    int index = static_cast<int>(s.getProxyType());
    bodyUI->proxyType->setCurrentIndex(index);
    on_proxyType_currentIndexChanged(index);

    eventsInit();
    Translator::registerHandler(std::bind(&AdvancedForm::retranslateUi, this), this);
}

AdvancedForm::~AdvancedForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void AdvancedForm::on_cbMakeToxPortable_stateChanged()
{
    Settings::getInstance().setMakeToxPortable(bodyUI->cbMakeToxPortable->isChecked());
}

void AdvancedForm::on_resetButton_clicked()
{
    const QString titile = tr("Reset settings");
    bool result = GUI::askQuestion(titile,
                                   tr("All settings will be reseted to deafult. Are you sure?"),
                                   tr("Yes"), tr("No"));

    if (!result)
        return;

    Settings::getInstance().resetToDefault();
    GUI::showInfo(titile, "Changes will take effect after restart");
}

void AdvancedForm::on_cbEnableIPv6_stateChanged()
{
    Settings::getInstance().setEnableIPv6(bodyUI->cbEnableIPv6->isChecked());
}

void AdvancedForm::on_cbEnableUDP_stateChanged()
{
    Settings::getInstance().setForceTCP(!bodyUI->cbEnableUDP->isChecked());
}

void AdvancedForm::on_proxyAddr_editingFinished()
{
    Settings::getInstance().setProxyAddr(bodyUI->proxyAddr->text());
}

void AdvancedForm::on_proxyPort_valueChanged(int port)
{
    if (port <= 0)
        port = 0;

    Settings::getInstance().setProxyPort(port);
}

void AdvancedForm::on_proxyType_currentIndexChanged(int index)
{
    Settings::ProxyType proxytype = static_cast<Settings::ProxyType>(index);

    bodyUI->proxyAddr->setEnabled(proxytype != Settings::ProxyType::ptNone);
    bodyUI->proxyPort->setEnabled(proxytype != Settings::ProxyType::ptNone);
    Settings::getInstance().setProxyType(proxytype);
}

void AdvancedForm::on_reconnectButton_clicked()
{
    if (Core::getInstance()->getAv()->anyActiveCalls())
    {
        QMessageBox::warning(this, tr("Call active", "popup title"),
                        tr("You can't disconnect while a call is active!", "popup text"));
        return;
    }

    emit Core::getInstance()->statusSet(Status::Offline);
    Nexus::getProfile()->restartCore();
}

void AdvancedForm::retranslateUi()
{
    int proxyType = bodyUI->proxyType->currentIndex();
    bodyUI->retranslateUi(this);
    bodyUI->proxyType->setCurrentIndex(proxyType);
}
