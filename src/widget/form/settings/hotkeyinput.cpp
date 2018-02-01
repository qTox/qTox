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
#include <QDebug>

#include "hotkeyinput.h"
#include "audio/iaudiosettings.h"
#include "src/persistence/settings.h"

HotkeyInput::HotkeyInput(QWidget* parent)
    : QTextEdit(parent)
{
    setAcceptRichText(false);
    setAcceptDrops(false);
}

void HotkeyInput::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    int nativeKey = event->nativeScanCode();
    QString keyString;
    QString modifierString = "";

    if (event->modifiers() & Qt::MetaModifier) {
        modifierString += "Command+";
    }

    if (event->modifiers() & Qt::ControlModifier) {
        modifierString += "Ctrl+";
    }

    if (event->modifiers() & Qt::ShiftModifier) {
        modifierString += "Shift+";
    }

    if (event->modifiers() & Qt::AltModifier) {
        modifierString += "Alt+";
    }

    QKeySequence keySequence = QKeySequence(nativeKey);
    keyString = modifierString;

    if (keySequence != Qt::Key_unknown && keySequence != 0) {
        keyString += keySequence.toString();
    }
    
    QList<int> keys = Settings::getInstance().getPttShortcutKeys();
    if (keys.indexOf(nativeKey) == -1)
    {
	    keys.append(nativeKey);
	    Settings::getInstance().setPttShortcutKeys(keys);
    }

    keyString.replace(QRegExp("++$"), "");
    this->setText(keyString);

    qDebug() << " " << event->nativeScanCode();
}
