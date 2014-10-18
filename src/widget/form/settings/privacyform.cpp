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

#include "privacyform.h"
#include "ui_privacysettings.h"
#include "src/widget/form/settingswidget.h"
#include "src/misc/settings.h"
#include "src/historykeeper.h"
#include "src/core.h"
#include "src/widget/widget.h"
#include "src/widget/form/setpassworddialog.h"

PrivacyForm::PrivacyForm() :
    GenericForm(tr("Privacy settings"), QPixmap(":/img/settings/privacy.png"))
{
    bodyUI = new Ui::PrivacySettings;
    bodyUI->setupUi(this);

    bodyUI->cbTypingNotification->setChecked(Settings::getInstance().isTypingNotificationEnabled());
    bodyUI->cbKeepHistory->setChecked(Settings::getInstance().getEnableLogging());
    bodyUI->cbEncryptHistory->setChecked(Settings::getInstance().getEncryptLogs());

    connect(bodyUI->cbTypingNotification, SIGNAL(stateChanged(int)), this, SLOT(onTypingNotificationEnabledUpdated()));
    connect(bodyUI->cbKeepHistory, SIGNAL(stateChanged(int)), this, SLOT(onEnableLoggingUpdated()));
    connect(bodyUI->cbEncryptHistory, SIGNAL(stateChanged(int)), this, SLOT(onEncryptLogsUpdated()));
    connect(bodyUI->cbEncryptTox, SIGNAL(stateChanged(int)), this, SLOT(onEncryptToxUpdated()));
}

PrivacyForm::~PrivacyForm()
{
    delete bodyUI;
}

void PrivacyForm::onEnableLoggingUpdated()
{
    Settings::getInstance().setEnableLogging(bodyUI->cbKeepHistory->isChecked());
    bodyUI->cbEncryptHistory->setEnabled(bodyUI->cbKeepHistory->isChecked());
    HistoryKeeper::getInstance()->resetInstance();
}

void PrivacyForm::onTypingNotificationEnabledUpdated()
{
    Settings::getInstance().setTypingNotification(bodyUI->cbTypingNotification->isChecked());
}

void PrivacyForm::onEncryptLogsUpdated()
{
    bool encpytionState = bodyUI->cbEncryptHistory->isChecked();

    if (encpytionState)
    {
        if (!Core::getInstance()->isPasswordSet())
        {
            SetPasswordDialog dialog;
            if (dialog.exec())
            {
                QString pswd = dialog.getPassword();
                if (pswd.size() > 0)
                {
                    Core::getInstance()->setPassword(pswd);
                    Settings::getInstance().setEncryptLogs(true);
                    return;
                }
            }

            bodyUI->cbEncryptHistory->setChecked(false);
            Settings::getInstance().setEncryptLogs(false);
        } else {
            Settings::getInstance().setEncryptLogs(true);
        }
    } else {
        Settings::getInstance().setEncryptLogs(false);
    }
}

void PrivacyForm::onEncryptToxUpdated()
{
    //
}
