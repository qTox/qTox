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

class Core;
class Settings;
class Style;
class IMessageBoxManager;

namespace Ui {
class AdvancedSettings;
}

class AdvancedForm : public GenericForm
{
    Q_OBJECT
public:
    AdvancedForm(Settings& settings, Style& style, IMessageBoxManager& messageBoxManager);
    ~AdvancedForm();
    QString getFormName() final
    {
        return tr("Advanced");
    }

private slots:
    // Portable
    void on_cbMakeToxPortable_stateChanged();
    void on_resetButton_clicked();
    // Debug
    void on_btnCopyDebug_clicked();
    void on_btnExportLog_clicked();
    // Connection
    void on_cbEnableIPv6_stateChanged();
    void on_cbEnableUDP_stateChanged();
    void on_cbEnableLanDiscovery_stateChanged();
    void on_proxyAddr_editingFinished();
    void on_proxyPort_valueChanged(int port);
    void on_proxyType_currentIndexChanged(int index);

private:
    void retranslateUi();

private:
    Ui::AdvancedSettings* bodyUI;
    Settings& settings;
    IMessageBoxManager& messageBoxManager;
};
