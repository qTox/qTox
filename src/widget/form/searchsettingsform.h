/*
    Copyright © 2019 by The qTox Project Contributors

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

#ifndef SEARCHSETTINGSFORM_H
#define SEARCHSETTINGSFORM_H

#include <QWidget>
#include "src/widget/searchtypes.h"

namespace Ui {
class SearchSettingsForm;
}

class SearchSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SearchSettingsForm(QWidget *parent = nullptr);
    ~SearchSettingsForm();

    ParameterSearch getParameterSearch();
    void reloadTheme();

private:
    Ui::SearchSettingsForm *ui;
    QDate startDate;
    bool isUpdate{false};

    void updateStartDateLabel();
    void setUpdate(const bool isUpdate);

private slots:
    void onStartSearchSelected(const int index);
    void onRegisterClicked(const bool checked);
    void onWordsOnlyClicked(const bool checked);
    void onRegularClicked(const bool checked);
    void onChoiceDate();

signals:
    void updateSettings(const bool isUpdate);
};

#endif // SEARCHSETTINGSFORM_H
