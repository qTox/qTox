/*
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

#ifndef PRIVACYFORM_H
#define PRIVACYFORM_H

#include "genericsettings.h"

namespace Ui {
class PrivacySettings;
}

class PrivacyForm : public GenericForm
{
    Q_OBJECT
public:
    PrivacyForm();
    ~PrivacyForm();
    QString getFormName() final {return tr("Privacy");}

private slots:
    void onEnableLoggingUpdated();
    void onTypingNotificationEnabledUpdated();
    void setNospam();
    void generateRandomNospam();
    void onNospamEdit();
    virtual void showEvent(QShowEvent*);

private:
    void retranslateUi();

private:
    Ui::PrivacySettings* bodyUI;
};

#endif
