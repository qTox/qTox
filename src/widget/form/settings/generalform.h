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

#pragma once

#include "genericsettings.h"

namespace Ui {
class GeneralSettings;
}

class SettingsWidget;

class GeneralForm : public GenericForm
{
    Q_OBJECT
public:
    explicit GeneralForm(SettingsWidget* parent);
    ~GeneralForm();
    QString getFormName() final
    {
        return tr("General");
    }
signals:
    void updateIcons();

private slots:
    void on_transComboBox_currentIndexChanged(int index);
    void on_cbAutorun_stateChanged();
    void on_cbSpellChecking_stateChanged();
    void on_showSystemTray_stateChanged();
    void on_startInTray_stateChanged();
    void on_closeToTray_stateChanged();
    void on_lightTrayIcon_stateChanged();
    void on_autoAwaySpinBox_editingFinished();
    void on_minimizeToTray_stateChanged();
    void on_statusChanges_stateChanged();
    void on_autoacceptFiles_stateChanged();
    void on_maxAutoAcceptSizeMB_editingFinished();
    void on_autoSaveFilesDir_clicked();
    void on_checkUpdates_stateChanged();

private:
    void retranslateUi();

private:
    Ui::GeneralSettings* bodyUI;
    SettingsWidget* parent;
};
