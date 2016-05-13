/*
    Copyright © 2016 by The qTox Project

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

#ifndef SETTINGSSAVETRANSACTION_H
#define SETTINGSSAVETRANSACTION_H

#include <QObject>
#include <QMutex>
#include <QSemaphore>

class SettingsTransaction : public QObject
{
    Q_OBJECT

public:
    explicit SettingsTransaction();
    bool canceled = false;
};

class PersonalSettingsTransaction : public SettingsTransaction
{
    Q_OBJECT

public:
    explicit PersonalSettingsTransaction();
    ~PersonalSettingsTransaction();
    void cancel();

private:
    static QSemaphore activeTransactions;
    static QMutex lock;
};

class GlobalSettingsTransaction : public SettingsTransaction
{
    Q_OBJECT

public:
    explicit GlobalSettingsTransaction();
    ~GlobalSettingsTransaction();
    void cancel();

private:
    static QSemaphore activeTransactions;
    static QMutex lock;
};

#endif // SETTINGSSAVETRANSACTION_H
