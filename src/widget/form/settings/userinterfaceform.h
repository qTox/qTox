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

#ifndef USERINTERFACEFORM_H
#define USERINTERFACEFORM_H

#include "genericsettings.h"
#include "src/widget/form/settingswidget.h"

namespace Ui {
class UserInterfaceSettings;
}

class UserInterfaceForm : public GenericForm
{
    Q_OBJECT
public:
    explicit UserInterfaceForm(SettingsWidget* myParent);
    ~UserInterfaceForm();
    virtual QString getFormName() final override {return tr("User Interface");}

private slots:
    void onSmileyBrowserIndexChanged(int index);
    void onEmoticonSizeChanged();
    void onStyleSelected(QString style);
    void onTimestampSelected(int index);
    void onDateFormatSelected(int index);
    void onStyleUpdated();
    void onUseEmoticonsChange();
    void onShowWindowChanged();
    void onSetShowInFront();
    void onSetGroupAlwaysNotify();
    void onCompactLayout();
    void onSeparateWindowChanged();
    void onDontGroupWindowsChanged();
    void onGroupchatPositionChanged();
    void onThemeColorChanged(int);

    void on_txtChatFont_currentFontChanged(const QFont& f);
    void on_txtChatFontSize_valueChanged(int arg1);

private:
    void retranslateUi();
    void reloadSmiles();

private:
    QList<QLabel*> smileLabels;
    SettingsWidget* parent;
    Ui::UserInterfaceSettings *bodyUI;
};

#endif
