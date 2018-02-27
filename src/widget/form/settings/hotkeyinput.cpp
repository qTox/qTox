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

bool wasCleared = false;
bool isReadyToOverwrite = false;
QString noKeyString = QObject::tr("<no key>");
QString pressAnyKeyString = QObject::tr("Press any key or Esc to cancel");

HotkeyInput::HotkeyInput(QWidget* parent)
    : QLineEdit(parent)
{
    setPlaceholderText(noKeyString);
    setReadOnly(true); // hides the caret
}

QString keyListToString(QList<int> keys)
{
    // TODO: get key names from GlobalHotkey instead of displaying key numbers
    QString keyString = "";
    for (int i = 0; i < keys.length(); i++) {
        keyString += QString::number(keys[i]) + "+";
    }

    keyString.replace(QRegExp("[+]+$"), "");
    return keyString;
}

void HotkeyInput::keyPressEvent(QKeyEvent* event)
{
    // we only want to act on key press, not on key hold
    if (event->isAutoRepeat()) {
        return;
    }

    QList<int> keys;

    if (event->key() == Qt::Key_Escape) {
        this->clear();
        wasCleared = true;
        Settings::getInstance().setPttShortcutKeys(keys);
        this->clearFocus();
        return;
    }

    int nativeKey = event->nativeScanCode();
    if (!isReadyToOverwrite) {
        keys = Settings::getInstance().getPttShortcutKeys();
    }

    if (keys.indexOf(nativeKey) == -1) {
        keys.append(nativeKey);
        Settings::getInstance().setPttShortcutKeys(keys);
    }

    isReadyToOverwrite = false;
    this->setText(keyListToString(keys));
}

void HotkeyInput::keyReleaseEvent(QKeyEvent* event)
{
    if (!event->isAutoRepeat()) {
        this->clearFocus();
    }
}

void HotkeyInput::focusInEvent(QFocusEvent* event)
{
    const QList<int> keys = Settings::getInstance().getPttShortcutKeys();
    this->clear();
    setPlaceholderText(pressAnyKeyString);
    isReadyToOverwrite = true;
}

void HotkeyInput::focusOutEvent(QFocusEvent* event)
{
    const QString text = this->text();
    if (text == "" && !wasCleared) {
        const QList<int> keys = Settings::getInstance().getPttShortcutKeys();
        this->setText(keyListToString(keys));
    }

    wasCleared = false;
    setPlaceholderText(noKeyString);
}
