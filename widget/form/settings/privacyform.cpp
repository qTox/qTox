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
#include "widget/form/settingswidget.h"
#include "misc/settings.h"

PrivacyForm::PrivacyForm() :
    GenericForm(tr("Privacy settings"), QPixmap(":/img/settings/privacy.png"))
{
    bodyUI = new Ui::PrivacySettings;
    bodyUI->setupUi(this);
}

PrivacyForm::~PrivacyForm()
{
    delete bodyUI;
}

void PrivacyForm::onEnableLoggingUpdated()
{
    Settings::getInstance().setEnableLogging(bodyUI->cbKeepHistory->isChecked());
    bodyUI->cbEncryptHistory->setEnabled(bodyUI->cbKeepHistory->isChecked());
}

void PrivacyForm::onTypingNotificationEnabledUpdated()
{
    Settings::getInstance().setTypingNotification(bodyUI->cbTypingNotification->isChecked());
}

void PrivacyForm::onEncryptLogsUpdated()
{
    Settings::getInstance().setEncryptLogs(bodyUI->cbEncryptHistory->isChecked());
}

void PrivacyForm::onEncryptToxUpdated()
{
    //
}

void PrivacyForm::onPasswordSet()
{
    //
}
