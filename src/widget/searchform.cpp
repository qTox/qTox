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

#include "searchform.h"
#include "form/searchsettingsform.h"
#include "src/widget/style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>

#include <array>

static std::array<QString, 3> STATE_NAME = {
    QString{},
    QStringLiteral("green"),
    QStringLiteral("red"),
};

SearchForm::SearchForm(Settings& settings_, Style& style_, QWidget* parent)
    : QWidget(parent)
    , settings{settings_}
    , style{style_}
{
    QVBoxLayout* layout = new QVBoxLayout();
    QHBoxLayout* layoutNavigation = new QHBoxLayout();
    QHBoxLayout* layoutMessage = new QHBoxLayout();
    QSpacerItem *lSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Ignored);
    QSpacerItem *rSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Ignored);
    searchLine = new LineEdit();
    searchSettingsForm = new SearchSettingsForm(settings, style);
    messageLabel = new QLabel();

    searchSettingsForm->setVisible(false);
    messageLabel->setProperty("state", QStringLiteral("red"));
    messageLabel->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/labels.css"),settings));
    messageLabel->setText(tr("The text could not be found."));
    messageLabel->setVisible(false);

    settingsButton = createButton("searchSettingsButton", "green");
    upButton = createButton("searchUpButton", "green");
    downButton = createButton("searchDownButton", "green");
    hideButton = createButton("searchHideButton", "red");
    startButton = createButton("startButton", "green");
    startButton->setText(tr("Start"));

    layoutNavigation->setMargin(0);
    layoutNavigation->addWidget(settingsButton);
    layoutNavigation->addWidget(searchLine);
    layoutNavigation->addWidget(startButton);
    layoutNavigation->addWidget(upButton);
    layoutNavigation->addWidget(downButton);
    layoutNavigation->addWidget(hideButton);

    layout->addLayout(layoutNavigation);
    layout->addWidget(searchSettingsForm);

    layoutMessage->addSpacerItem(lSpacer);
    layoutMessage->addWidget(messageLabel);
    layoutMessage->addSpacerItem(rSpacer);
    layout->addLayout(layoutMessage);

    startButton->setHidden(true);

    setLayout(layout);

    connect(searchLine, &LineEdit::textChanged, this, &SearchForm::changedSearchPhrase);
    connect(searchLine, &LineEdit::clickEnter, this, &SearchForm::clickedUp);
    connect(searchLine, &LineEdit::clickShiftEnter, this, &SearchForm::clickedDown);
    connect(searchLine, &LineEdit::clickEsc, this, &SearchForm::clickedHide);

    connect(upButton, &QPushButton::clicked, this, &SearchForm::clickedUp);
    connect(downButton, &QPushButton::clicked, this, &SearchForm::clickedDown);
    connect(hideButton, &QPushButton::clicked, this, &SearchForm::clickedHide);
    connect(startButton, &QPushButton::clicked, this, &SearchForm::clickedStart);
    connect(settingsButton, &QPushButton::clicked, this, &SearchForm::clickedSearch);

    connect(searchSettingsForm, &SearchSettingsForm::updateSettings, this, &SearchForm::changedState);

    connect(&style, &Style::themeReload, this, &SearchForm::reloadTheme);
}

void SearchForm::removeSearchPhrase()
{
    searchLine->setText("");
}

QString SearchForm::getSearchPhrase() const
{
    return searchPhrase;
}

ParameterSearch SearchForm::getParameterSearch()
{
    return parameter;
}

void SearchForm::setFocusEditor()
{
    searchLine->setFocus();
}

void SearchForm::insertEditor(const QString &text)
{
    searchLine->insert(text);
}

void SearchForm::reloadTheme()
{
    settingsButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));
    upButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));
    downButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));
    hideButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));
    startButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));

    searchSettingsForm->reloadTheme();
}

void SearchForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit visibleChanged();
}

QPushButton *SearchForm::createButton(const QString& name, const QString& state)
{
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", state);
    btn->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));

    return btn;
}

ParameterSearch SearchForm::getAndCheckParametrSearch()
{
    if (isActiveSettings) {
        auto sendParam = searchSettingsForm->getParameterSearch();
        if (!isChangedPhrase && !sendParam.isUpdate) {
            sendParam.period = PeriodSearch::None;
        }

        isChangedPhrase = false;
        parameter = sendParam;

        return sendParam;
    }

    return ParameterSearch();
}

void SearchForm::setStateName(QPushButton *btn, ToolButtonState state)
{
    const auto index = static_cast<unsigned long>(state);
    btn->setProperty("state", STATE_NAME[index]);
    btn->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));
    btn->setEnabled(index != 0);
}

void SearchForm::useBeginState()
{
    setStateName(upButton, ToolButtonState::Common);
    setStateName(downButton, ToolButtonState::Common);
    messageLabel->setVisible(false);
    isPrevSearch = false;
}

void SearchForm::changedSearchPhrase(const QString& text)
{
    useBeginState();

    if (searchPhrase == text) {
        return;
    }

    QString l = text.right(1);
    if (!l.isEmpty() && l != " " && l[0].isSpace()) {
        searchLine->setText(searchPhrase);
        return;
    }

    searchPhrase = text;
    isChangedPhrase = true;
    if (isActiveSettings) {
        if (startButton->isHidden()) {
            changedState(true);
        }
    } else {
        isSearchInBegin = true;
        emit searchInBegin(searchPhrase, getAndCheckParametrSearch());
    }
}

void SearchForm::clickedUp()
{
    if (downButton->isEnabled()) {
        isPrevSearch = false;
    } else {
        isPrevSearch = true;
        setStateName(downButton, ToolButtonState::Common);
        messageLabel->setVisible(false);
    }

    if (startButton->isHidden()) {
        isSearchInBegin = false;
        emit searchUp(searchPhrase, getAndCheckParametrSearch());
    } else {
        clickedStart();
    }
}

void SearchForm::clickedDown()
{
    if (upButton->isEnabled()) {
        isPrevSearch = false;
    } else {
        isPrevSearch = true;
        setStateName(upButton, ToolButtonState::Common);
        messageLabel->setVisible(false);
    }

    if (startButton->isHidden()) {
        isSearchInBegin = false;
        emit searchDown(searchPhrase, getAndCheckParametrSearch());
    } else {
        clickedStart();
    }
}

void SearchForm::clickedHide()
{
    hide();
    emit visibleChanged();
}

void SearchForm::clickedStart()
{
    changedState(false);
    isSearchInBegin = true;
    emit searchInBegin(searchPhrase, getAndCheckParametrSearch());
}

void SearchForm::clickedSearch()
{
    isActiveSettings = !isActiveSettings;
    searchSettingsForm->setVisible(isActiveSettings);
    useBeginState();

    if (isActiveSettings) {
        setStateName(settingsButton, ToolButtonState::Active);
    } else {
        setStateName(settingsButton, ToolButtonState::Common);
        changedState(false);
    }
}

void SearchForm::changedState(bool isUpdate)
{
    if (isUpdate) {
        startButton->setHidden(false);
        upButton->setHidden(true);
        downButton->setHidden(true);
    } else {
        startButton->setHidden(true);
        upButton->setHidden(false);
        downButton->setHidden(false);
    }

    useBeginState();
}

void SearchForm::showMessageNotFound(SearchDirection direction)
{
    if (isSearchInBegin) {
        if (parameter.period == PeriodSearch::AfterDate) {
            setStateName(downButton, ToolButtonState::Disabled);
        } else if (parameter.period == PeriodSearch::BeforeDate) {
            setStateName(upButton, ToolButtonState::Disabled);
        } else {
            setStateName(upButton, ToolButtonState::Disabled);
            setStateName(downButton, ToolButtonState::Disabled);
        }
    } else if (isPrevSearch) {
        setStateName(upButton, ToolButtonState::Disabled);
        setStateName(downButton, ToolButtonState::Disabled);
    } else if (direction == SearchDirection::Up) {
        setStateName(upButton, ToolButtonState::Disabled);
    } else {
        setStateName(downButton, ToolButtonState::Disabled);
    }
    messageLabel->setVisible(true);
}

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent)
{
}

void LineEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();

    if ((key == Qt::Key_Enter || key == Qt::Key_Return)) {
        if ((event->modifiers() & Qt::ShiftModifier)) {
            emit clickShiftEnter();
        } else {
            emit clickEnter();
        }
    } else if (key == Qt::Key_Escape) {
        emit clickEsc();
    }

    QLineEdit::keyPressEvent(event);
}
