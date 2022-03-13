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

#include <QDialog>

class Style;

class ActivateDialog : public QDialog
{
    Q_OBJECT
public:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    ActivateDialog(Style& style, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
#else
    ActivateDialog(Style& style, QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
#endif
    bool event(QEvent* event) override;

public slots:
    virtual void reloadTheme() {}

signals:
    void windowStateChanged(Qt::WindowStates state);
};
