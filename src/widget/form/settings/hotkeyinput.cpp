/*
    Copyright © 2018-2020 by The qTox Project Contributors

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

#include <QKeyEvent>
#include <QObject>

#include "hotkeyinput.h"
#include "audio/iaudiosettings.h"
#include "src/persistence/settings.h"

namespace
{
    static const QString noKeyString = QObject::tr("<no key>");
    static const QString pressAnyKeyString = QObject::tr("Press any key or Esc to cancel");

    int healUnprintableKeys(int key)
    {
        switch (key)
        {
        case Qt::Key_Meta:
            return Qt::MetaModifier;
        case Qt::Key_Shift:
            return Qt::ShiftModifier;
        case Qt::Key_Control:
            return Qt::ControlModifier;
        case Qt::Key_Alt:
            return Qt::AltModifier;
        }
        return key;
    }

    QString stripTrailingPlus(QString keyName)
    {
        // for some reason Qt's name of modifier keys end in a plus, even by themselves
        if (keyName.size() > 1 && keyName.endsWith('+'))
        {
            return keyName.left(keyName.size() - 1);
        }
        return keyName;
    }

    QString keysToLabel(QList<int> keyNames)
    {
        QString presentedKeyName;
        const auto numKeys = keyNames.length();
        for (auto idx = 0; idx < numKeys; ++idx)
        {
            const auto keyName = stripTrailingPlus(QKeySequence(keyNames[idx]).toString());
            presentedKeyName += keyName;
            if (idx != numKeys-1) {
                presentedKeyName += "+";
            }
        }
        return presentedKeyName;
    }
}

HotkeyInput::HotkeyInput(QWidget* parent)
    : QLineEdit(parent)
{
    setPlaceholderText(noKeyString);
    setReadOnly(true); // hides the caret
}

void HotkeyInput::Initialize(IAudioSettings& _settings)
{
    settings = &_settings;
    const QList<int> keyNames = settings->getPttShortcutNames();
    this->setText(keysToLabel(keyNames));
}

void HotkeyInput::keyPressEvent(QKeyEvent* event)
{
    // we only want to act on key press, not on key hold
    if (event->isAutoRepeat()) {
        return;
    }

    QList<int> keyVals;
    QList<int> keyNames;

    if (event->key() == Qt::Key_Escape) {
        this->clear();
        wasCleared = true;
        settings->setPttShortcutKeys(keyVals);
        settings->setPttShortcutNames(keyNames);
        this->clearFocus();
        return;
    }

    int nativeKey = event->nativeScanCode();
    if (!isReadyToOverwrite) {
        keyVals = settings->getPttShortcutKeys();
        keyNames = settings->getPttShortcutNames();
    }

    int healedKey;
    if (keyVals.indexOf(nativeKey) == -1) {
        keyVals.append(nativeKey);
        settings->setPttShortcutKeys(keyVals);
        healedKey = healUnprintableKeys(event->key());
        keyNames.append(healedKey);
        settings->setPttShortcutNames(keyNames);
    }

    isReadyToOverwrite = false;
    this->setText(keysToLabel(keyNames));
}

void HotkeyInput::keyReleaseEvent(QKeyEvent* event)
{
    if (!event->isAutoRepeat()) {
        this->clearFocus();
    }
}

void HotkeyInput::focusInEvent(QFocusEvent* event)
{
    this->clear();
    setPlaceholderText(pressAnyKeyString);
    isReadyToOverwrite = true;
}

void HotkeyInput::focusOutEvent(QFocusEvent* event)
{
    const QString text = this->text();
    if (text == "" && !wasCleared) {
        const QList<int> keyNames = settings->getPttShortcutNames();
        this->setText(keysToLabel(keyNames));
    }

    wasCleared = false;
    setPlaceholderText(noKeyString);
}
