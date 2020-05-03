/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include <QWidget>
#include <QLineEdit>
#include "searchtypes.h"

class QPushButton;
class QLabel;
class LineEdit;
class SearchSettingsForm;

class SearchForm final : public QWidget
{
    Q_OBJECT
public:
    enum class ToolButtonState {
        Disabled = 0,    // Grey
        Common = 1,      // Green
        Active = 2,      // Red
    };

    explicit SearchForm(QWidget* parent = nullptr);
    void removeSearchPhrase();
    QString getSearchPhrase() const;
    ParameterSearch getParameterSearch();
    void setFocusEditor();
    void insertEditor(const QString &text);
    void reloadTheme();

protected:
    void showEvent(QShowEvent* event) final;

private:
    // TODO: Merge with 'createButton' from chatformheader.cpp
    QPushButton* createButton(const QString& name, const QString& state);
    ParameterSearch getAndCheckParametrSearch();
    void setStateName(QPushButton* btn, ToolButtonState state);
    void useBeginState();

    QPushButton* settingsButton;
    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* hideButton;
    QPushButton* startButton;
    LineEdit* searchLine;
    SearchSettingsForm* settings;
    QLabel* messageLabel;

    QString searchPhrase;
    ParameterSearch parameter;

    bool isActiveSettings{false};
    bool isChangedPhrase{false};
    bool isSearchInBegin{true};
    bool isPrevSearch{false};

private slots:
    void changedSearchPhrase(const QString& text);
    void clickedUp();
    void clickedDown();
    void clickedHide();
    void clickedStart();
    void clickedSearch();
    void changedState(bool isUpdate);

public slots:
    void showMessageNotFound(SearchDirection direction);

signals:
    void searchInBegin(const QString& phrase, const ParameterSearch& parameter);
    void searchUp(const QString& phrase, const ParameterSearch& parameter);
    void searchDown(const QString& phrase, const ParameterSearch& parameter);
    void visibleChanged();
};

class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) final;

signals:
    void clickEnter();
    void clickShiftEnter();
    void clickEsc();
};
