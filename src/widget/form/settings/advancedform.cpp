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

#include <QMessageBox>

#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/recursivesignalblocker.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/persistence/db/plaindb.h"
#include "src/persistence/profile.h"
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
    int port = s.getProxyPort();
    if (port != -1)
        bodyUI->proxyPort->setValue(port);

    bodyUI->proxyType->setCurrentIndex(static_cast<int>(s.getProxyType()));
    onUseProxyUpdated();

    // portable
    connect(bodyUI->cbMakeToxPortable, &QCheckBox::stateChanged, this, &AdvancedForm::onMakeToxPortableUpdated);
    connect(bodyUI->resetButton, &QPushButton::clicked, this, &AdvancedForm::resetToDefault);
    //connection
    void (QComboBox::* currentIndexChanged)(int) = &QComboBox::currentIndexChanged;
    void (QSpinBox::* valueChanged)(int) = &QSpinBox::valueChanged;
    connect(bodyUI->cbEnableIPv6, &QCheckBox::stateChanged, this, &AdvancedForm::onEnableIPv6Updated);
    connect(bodyUI->cbEnableUDP, &QCheckBox::stateChanged, this, &AdvancedForm::onUDPUpdated);
    connect(bodyUI->proxyType, currentIndexChanged, this, &AdvancedForm::onUseProxyUpdated);
    connect(bodyUI->proxyAddr, &QLineEdit::editingFinished, this, &AdvancedForm::onProxyAddrEdited);
    connect(bodyUI->proxyPort, valueChanged, this, &AdvancedForm::onProxyPortEdited);
    connect(bodyUI->reconnectButton, &QPushButton::clicked, this, &AdvancedForm::onReconnectClicked);

    for (QCheckBox *cb : findChildren<QCheckBox*>()) // this one is to allow scrolling on checkboxes
        cb->installEventFilter(this);

    Translator::registerHandler(std::bind(&AdvancedForm::retranslateUi, this), this);
}

AdvancedForm::~AdvancedForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void AdvancedForm::onMakeToxPortableUpdated()
{
    Settings::getInstance().setMakeToxPortable(bodyUI->cbMakeToxPortable->isChecked());
}

void AdvancedForm::resetToDefault()
{
}

void AdvancedForm::onEnableIPv6Updated()
{
    Settings::getInstance().setEnableIPv6(bodyUI->cbEnableIPv6->isChecked());
}

void AdvancedForm::onUDPUpdated()
{
    Settings::getInstance().setForceTCP(!bodyUI->cbEnableUDP->isChecked());
}

void AdvancedForm::onProxyAddrEdited()
{
    Settings::getInstance().setProxyAddr(bodyUI->proxyAddr->text());
}

void AdvancedForm::onProxyPortEdited(int port)
{
    if (port <= 0)
        port = -1;

    Settings::getInstance().setProxyPort(port);
}

void AdvancedForm::onUseProxyUpdated()
{
    Settings::ProxyType proxytype =
            static_cast<Settings::ProxyType>(bodyUI->proxyType->currentIndex());

    bodyUI->proxyAddr->setEnabled(proxytype != Settings::ProxyType::ptNone);
    bodyUI->proxyPort->setEnabled(proxytype != Settings::ProxyType::ptNone);
    Settings::getInstance().setProxyType(proxytype);
}

void AdvancedForm::onReconnectClicked()
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
