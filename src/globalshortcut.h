/*
    Copyright © 2020 by The qTox Project Contributors

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

#include <memory>

#include <QObject>
#include <QMutex>

#include "src/hookmanager.h"

class QThread;

class GlobalShortcut : public QObject
{
Q_OBJECT
public:
    GlobalShortcut(QObject *parent = 0);
    ~GlobalShortcut();
    std::unique_ptr<std::vector<int>> getShortcutKeys();

public slots:
    void onPttShortcutKeysChanged(QList<int> keys);

signals:
    void toggled();

private:
    std::unique_ptr<HookManager> hookManager;
    QThread* thread;
    std::vector<int> shortcutKeys;
    QMutex keysMutex;
};
