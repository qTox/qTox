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

#ifndef GENERALFORM_H
#define GENERALFORM_H

#include "genericsettings.h"

namespace Ui {
class GeneralSettings;
}

class SettingsWidget;

class GeneralForm : public GenericForm
{
    Q_OBJECT
public:
    explicit GeneralForm(SettingsWidget *parent);
    ~GeneralForm();
    virtual QString getFormName() final override
    {
        return tr("General");
    }

private slots:
    void onTranslationUpdated();
    void onAutorunUpdated();
    void onSetShowSystemTray();
    void onSetAutostartInTray();
    void onSetCloseToTray();
    void onSetLightTrayIcon();
    void onAutoAwayChanged();
    void onSetMinimizeToTray();
    void onSetNotifySound();
    void onSetBusySound();
    void onSetStatusChange();
    void onFauxOfflineMessaging();

    void onAutoAcceptFileChange();
    void onAutoSaveDirChange();
    void onCheckUpdateChanged();

private:
    void retranslateUi();

private:
    Ui::GeneralSettings *bodyUI;
    SettingsWidget *parent;
};

#endif
