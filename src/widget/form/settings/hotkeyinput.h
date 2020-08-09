/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#ifndef HOTKEYINPUT_H
#define HOTKEYINPUT_H

#include <QLineEdit>

class HotkeyInput : public QLineEdit
{
    Q_OBJECT
public:
    explicit HotkeyInput(QWidget* parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent* event) final override;
    virtual void keyReleaseEvent(QKeyEvent* event) final override;
    virtual void focusInEvent(QFocusEvent* event) final override;
    virtual void focusOutEvent(QFocusEvent* event) final override;
};

#endif // HOTKEYINPUT_H
