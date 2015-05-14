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
    GeneralForm(SettingsWidget *parent);
    ~GeneralForm();

private slots:
    void onEnableIPv6Updated();
    void onTranslationUpdated();
    void onAutorunUpdated();
    void onSetShowSystemTray();
    void onSetAutostartInTray();
    void onSetCloseToTray();
    void onSetLightTrayIcon();
    void onSmileyBrowserIndexChanged(int index);
    void onUDPUpdated();
    void onProxyAddrEdited();
    void onProxyPortEdited(int port);
    void onUseProxyUpdated();
    void onEmoticonSizeChanged();
    void onStyleSelected(QString style);  
    void onTimestampSelected(int index);
    void onDateFormatSelected(int index);
    void onSetStatusChange();
    void onAutoAwayChanged();
    void onUseEmoticonsChange();
    void onSetMinimizeToTray();
    void onReconnectClicked();
    void onAutoAcceptFileChange();
    void onAutoSaveDirChange();
    void onCheckUpdateChanged();
    void onShowWindowChanged();
    void onSetShowInFront();
    void onSetNotifySound();
    void onSetGroupAlwaysNotify();
    void onFauxOfflineMessaging();
    void onCompactLayout();
    void onGroupchatPositionChanged();
    void onThemeColorChanged(int);

private:
    Ui::GeneralSettings *bodyUI;
    void reloadSmiles();
    SettingsWidget *parent;

protected:
    bool eventFilter(QObject *o, QEvent *e);
};

#endif
